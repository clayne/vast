// Copyright (c) 2024, Trail of Bits, Inc.

#ifndef VAST_DIALECT_PARSER_OPS
#define VAST_DIALECT_PARSER_OPS

include "mlir/IR/OpBase.td"

include "vast/Dialect/Core/Utils.td"
include "vast/Dialect/Core/Interfaces/SymbolInterface.td"
include "vast/Dialect/Parser/Types.td"

class Parser_Op< string mnemonic, list< Trait > traits = [] >
  : Op< Parser_Dialect, mnemonic, traits >;

def Parser_Source
  : Parser_Op< "source" >
  , Arguments< (ins Variadic<Parser_AnyDataType>:$arguments) >
  , Results< (outs Variadic<Parser_AnyDataType>:$result) >
{
  let summary = "Source of parsed data.";

  let hasFolder = 1;

  let assemblyFormat = [{
    $arguments attr-dict `:` functional-type($arguments, $result)
  }];
}

def Paser_Sink
  : Parser_Op< "sink" >
  , Arguments< (ins Variadic<Parser_AnyDataType>:$arguments) >
  , Results< (outs Variadic<Parser_NoDataType>:$result) >
{
  let summary = "Sink of parsed data.";

  let hasFolder = 1;

  let assemblyFormat = [{
    $arguments attr-dict `:` functional-type($arguments, $result)
  }];
}

def Parser_Parse
  : Parser_Op< "parse" >
  , Arguments< (ins Variadic<Parser_AnyDataType>:$arguments) >
  , Results< (outs Variadic<Parser_AnyDataType>:$result) >
{
  let summary = "Parsing operation data.";

  let hasFolder = 1;

  let assemblyFormat = [{
    $arguments attr-dict `:` functional-type($arguments, $result)
  }];
}

def Parse_NoParse
  : Parser_Op< "noparse" >
  , Arguments< (ins Variadic<Parser_NoDataType>:$arguments) >
  , Results< (outs Variadic<Parser_NoDataType>:$result) >
{
  let summary = "Non-parsing operation data.";

  let hasFolder = 1;

  let assemblyFormat = [{
    $arguments attr-dict `:` functional-type($arguments, $result)
  }];
}

def Parse_MaybeParse
  : Parser_Op< "maybeparse" >
  , Arguments< (ins Variadic<Parser_AnyDataType>:$arguments) >
  , Results< (outs Variadic<Parser_AnyDataType>:$result) >
{
  let summary = "Maybe parsing operation data.";

  let hasFolder = 1;

  let assemblyFormat = [{
    $arguments attr-dict `:` functional-type($arguments, $result)
  }];
}

def Parse_Cast
  : Parser_Op< "cast" >
  , Arguments< (ins Parser_AnyDataType:$operand) >
  , Results< (outs Parser_AnyDataType:$result) >
{
  let summary = "Casting operation.";

  let hasFolder = 1;

  let assemblyFormat = [{
    $operand attr-dict `:` functional-type($operand, $result)
  }];
}

def Parse_Decl
  : Parser_Op< "decl", [Core_VarSymbol] >
  , Arguments< (ins
    SymbolNameAttr:$sym_name,
    TypeAttr:$type
  ) >
{
  let summary = "Declaration operation.";

  let assemblyFormat = [{
    $sym_name attr-dict `:` $type
  }];
}

def Parse_Ref
  : Parser_Op< "ref" >
  , Arguments< (ins FlatSymbolRefAttr:$name) >
  , Results< (outs Parser_AnyDataType:$result) >
{
  let summary = "Parser variable reference operation.";

  let assemblyFormat = [{
    $name attr-dict `:` type($result)
  }];
}

// TODO: Types should match
def Parse_Assign
  : Parser_Op< "assign" >
  , Arguments< (ins
    Parser_AnyDataType:$value,
    Parser_AnyDataType:$target
  ) >
{
  let summary = "Assignment to referenced value.";

  let assemblyFormat = [{
    $value `to` $target attr-dict functional-type($value, $target)
  }];
}

#endif // VAST_DIALECT_PARSER_OPS
