// Copyright (c) 2022-present, Trail of Bits, Inc.

#pragma once

#include "vast/Util/Common.hpp"

VAST_RELAX_WARNINGS
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
VAST_UNRELAX_WARNINGS

#include "vast/Tower/Handle.hpp"
#include "vast/Tower/PassUtils.hpp"

#include <algorithm>
#include <deque>
#include <numeric>
#include <optional>
#include <unordered_map>
#include <vector>

// Workaround to allow ABI breaking changes in join_view
#ifdef _LIBCPP_ENABLE_EXPERIMENTAL
    #define _LIBCPP_ENABLE_EXPERIMENTAL_WAS_DEFINED
#endif

#if _LIBCPP_VERSION
    #define _LIBCPP_ENABLE_EXPERIMENTAL
#endif

#include <ranges>

#ifndef _LIBCPP_ENABLE_EXPERIMENTAL_WAS_DEFINED
    #if _LIBCPP_VERSION
        #undef _LIBCPP_ENABLE_EXPERIMENTAL
    #endif
#endif

namespace vast::tw {

    template< typename module_key_t >
    struct conversion_tree
    {
        using node_key_t       = std::size_t;
        using maybe_node_key_t = std::optional< node_key_t >;

        struct node
        {
            // So `mlir::Pass` does not expose an api to query its options. However,
            // it should print them when we ask for textual repr. So for now we do compare
            // those strings.
            // TODO: Some other unique key that is cheaper?
            using pass_key_t = std::string;

            // The user is responsible for being able to retrive the module using this
            // piece of data..
            module_key_t module_key;

            std::unordered_map< pass_key_t, node_key_t > next;

            explicit node(module_key_t key) : module_key(std::move(key)) {}

            void add_edge(pass_key_t pass_name, node_key_t idx) {
                next.emplace(std::move(pass_name), idx);
            }

            void add_edge(pass_ptr pass, node_key_t idx) { add_edge(to_string(pass), idx); }

            maybe_node_key_t get_next(const pass_key_t &key) const {
                if (auto it = next.find(key); it != next.end()) {
                    return it->second;
                }
                return std::nullopt;
            }

            maybe_node_key_t get_next(pass_ptr pass) const { return get_next(to_string(pass)); }
        };

        // Using a `std::vector` leads to a weird bug caught by asan even though we do not use
        // pointers but indices instead.
        std::deque< node > nodes;

      protected:
        maybe_node_key_t lookup_node(handle_t handle) const {
            auto it = std::ranges::find_if(nodes, [&](const node &n) {
                return n.module_key == handle.id;
            });

            if (it != nodes.end()) {
                return maybe_node_key_t { std::distance(nodes.begin(), it) };
            }
            return std::nullopt;
        }

        node_key_t lookup(
            conversion_passes_t &prefix, conversion_passes_t &suffix,
            node_key_t root, auto on_visit
        ) const {
            if (suffix.empty()) {
                return root;
            }

            const auto &top = suffix.back();

            auto maybe_next = nodes[root].get_next(top);
            // There is no outgoing edge, so we stop.
            if (!maybe_next) {
                return root;
            }

            // Callback to current index
            on_visit(root);

            // We found a way forward, so we modify the paths and recurse
            prefix.push_back(top);
            suffix.pop_back();
            return lookup(prefix, suffix, *maybe_next, on_visit);
        }

        auto lookup_prefix_(conversion_passes_t path, node_key_t start_at, auto on_visit) const
            -> std::tuple< node_key_t, conversion_passes_t >
        {
            conversion_passes_t prefix;

            // We will pop from front, so we reverse go get the cheaper `pop_back()`.
            std::ranges::reverse(path);
            auto last = lookup(prefix, path, start_at, on_visit);
            std::ranges::reverse(path);

            return std::make_tuple(last, std::move(path));
        }

        node_key_t mk_node(module_key_t key) {
            nodes.emplace_back(std::move(key));
            return nodes.size() - 1;
        }

      public:
        // Return key to the last module and the remainder of the path that needs to applied on
        // it.
        auto lookup_prefix(conversion_passes_t path, handle_t start_at_handle, auto on_visit)
            const -> std::tuple< module_key_t, conversion_passes_t > {
            auto maybe_start_idx = lookup_node(start_at_handle);
            auto start_idx       = maybe_start_idx ? *maybe_start_idx : 0;
            auto [idx, suffix]   = lookup_prefix_(std::move(path), start_idx, on_visit);
            return std::make_tuple(nodes[idx].module_key, std::move(suffix));
        }

        auto lookup_prefix(conversion_passes_t path, handle_t start_at_handle) const {
            return lookup_prefix(std::move(path), start_at_handle, [](auto) {});
        }

        void store(const conversion_passes_t &path, module_key_t key) {
            auto [last, suffix] = lookup_prefix_(path, 0, [](auto) {});

            // Root case
            if (suffix.empty()) {
                VAST_ASSERT(path.empty() && key == 0);
                mk_node(key);
                return;
            }

            // Currently each node has to have a module, so if a store is hapening, it is going
            // to create exactly one new edge.
            VAST_CHECK(
                suffix.size() == 1,
                "Trying to store module not exactly on edge, but: {0} away.", suffix.size()
            );
            nodes[last].add_edge(suffix.front(), mk_node(key));
        }

        bool present(const conversion_passes_t &path) const {
            auto [_, suffix] = lookup_prefix(path);
            return suffix.empty();
        }
    };

    struct module_storage
    {
        using module_key_t = handle_id_t;

      protected:
        // TODO: API-wise, we probably want to accept any type that is `mlir::OwningOpRef< T >`?
        handle_t store_module(owning_mlir_module_ref mod) {
            auto id      = next_id++;
            auto [it, _] = storage.insert({ id, std::move(mod) });
            return { id, it->second.get() };
        }

        handle_t get(module_key_t module_key) const {
            auto it = storage.find(module_key);
            VAST_CHECK(it != storage.end(), "Required module not found in the storage!");
            return { module_key, it->second.get() };
        }

      public:
        std::optional< handle_t > get(const conversion_passes_t &path, handle_t root) {
            if (auto [module_key, suffix] = trie.lookup_prefix(path, root); suffix.empty()) {
                return { get(module_key) };
            }
            return std::nullopt;
        }

        auto get_maximum_prefix_path(const conversion_passes_t &path, handle_t root) const
            -> std::tuple< std::vector< handle_t >, conversion_passes_t >
        {
            std::vector< handle_t > handles;
            auto yield = [&](module_key_t module_key) { handles.push_back(get(module_key)); };

            auto [_, suffix] = trie.lookup_prefix(path, root, yield);
            return std::make_tuple(std::move(handles), std::move(suffix));
        }

        handle_t store(const conversion_passes_t &path, owning_mlir_module_ref mod) {
            // Just store the module, so it gets assigned id.
            auto handle = store_module(std::move(mod));
            // Now add it to the trie.
            trie.store(path, handle.id);
            return handle;
        }

        // TODO: Does it even make sense to remove things explicitly by the user?
        void remove(handle_t) { VAST_UNIMPLEMENTED; }

      private:
        module_key_t next_id = 0;
        llvm::DenseMap< handle_id_t, owning_mlir_module_ref > storage;
        conversion_tree< module_key_t > trie;
    };
} // namespace vast::tw
