// Copyright (c) 2024, Trail of Bits, Inc.

#ifndef VAST_DIALECT_PARSER_TYPES
#define VAST_DIALECT_PARSER_TYPES

class Parser_Type< string name, string _mnemonic, list<Trait> traits = [] >
    : TypeDef< Parser_Dialect, name, traits >
{
    let mnemonic = _mnemonic;
}

def Parser_DataType : Parser_Type< "Data", "data" > {
    let summary = "Data type.";
}

def Parser_NoDataType : Parser_Type< "NoData", "nodata" > {
    let summary = "No data type.";
}

def Parser_MaybeDataType : Parser_Type< "MaybeData", "maybedata" > {
    let summary = "Maybe data type.";
}

def Parser_AnyDataType : AnyTypeOf< [
    Parser_NoDataType,
    Parser_DataType,
    Parser_MaybeDataType
]>;

#endif // VAST_DIALECT_PARSER_TYPES
