#if !defined(FCODER_LEX_GEN_HAND_WRITTEN_TYPES)
#define FCODER_LEX_GEN_HAND_WRITTEN_TYPES

struct Lexeme_Table_Value{
    Token_Base_Kind base_kind;
    u16 sub_kind;
};

struct Lexeme_Table_Lookup{
    b32 found_match;
    Token_Base_Kind base_kind;
    u16 sub_kind;
};

#endif
typedef u16 Token_Cpp_Kind;
enum{
TokenCppKind_EOF = 0,
TokenCppKind_Whitespace = 1,
TokenCppKind_LexError = 2,
TokenCppKind_BlockComment = 3,
TokenCppKind_LineComment = 4,
TokenCppKind_Backslash = 5,
TokenCppKind_LiteralInteger = 6,
TokenCppKind_LiteralIntegerU = 7,
TokenCppKind_LiteralIntegerL = 8,
TokenCppKind_LiteralIntegerUL = 9,
TokenCppKind_LiteralIntegerLL = 10,
TokenCppKind_LiteralIntegerULL = 11,
TokenCppKind_LiteralIntegerHex = 12,
TokenCppKind_LiteralIntegerHexU = 13,
TokenCppKind_LiteralIntegerHexL = 14,
TokenCppKind_LiteralIntegerHexUL = 15,
TokenCppKind_LiteralIntegerHexLL = 16,
TokenCppKind_LiteralIntegerHexULL = 17,
TokenCppKind_LiteralIntegerOct = 18,
TokenCppKind_LiteralIntegerOctU = 19,
TokenCppKind_LiteralIntegerOctL = 20,
TokenCppKind_LiteralIntegerOctUL = 21,
TokenCppKind_LiteralIntegerOctLL = 22,
TokenCppKind_LiteralIntegerOctULL = 23,
TokenCppKind_LiteralFloat32 = 24,
TokenCppKind_LiteralFloat64 = 25,
TokenCppKind_LiteralString = 26,
TokenCppKind_LiteralStringWide = 27,
TokenCppKind_LiteralStringUTF8 = 28,
TokenCppKind_LiteralStringUTF16 = 29,
TokenCppKind_LiteralStringUTF32 = 30,
TokenCppKind_LiteralStringRaw = 31,
TokenCppKind_LiteralStringWideRaw = 32,
TokenCppKind_LiteralStringUTF8Raw = 33,
TokenCppKind_LiteralStringUTF16Raw = 34,
TokenCppKind_LiteralStringUTF32Raw = 35,
TokenCppKind_LiteralCharacter = 36,
TokenCppKind_LiteralCharacterWide = 37,
TokenCppKind_LiteralCharacterUTF8 = 38,
TokenCppKind_LiteralCharacterUTF16 = 39,
TokenCppKind_LiteralCharacterUTF32 = 40,
TokenCppKind_PPIncludeFile = 41,
TokenCppKind_PPErrorMessage = 42,
TokenCppKind_KeywordGeneric = 43,
TokenCppKind_BraceOp = 44,
TokenCppKind_BraceCl = 45,
TokenCppKind_ParenOp = 46,
TokenCppKind_BrackOp = 47,
TokenCppKind_ParenCl = 48,
TokenCppKind_BrackCl = 49,
TokenCppKind_Semicolon = 50,
TokenCppKind_Colon = 51,
TokenCppKind_DotDotDot = 52,
TokenCppKind_ColonColon = 53,
TokenCppKind_PlusPlus = 54,
TokenCppKind_MinusMinus = 55,
TokenCppKind_Dot = 56,
TokenCppKind_Arrow = 57,
TokenCppKind_Plus = 58,
TokenCppKind_Minus = 59,
TokenCppKind_Not = 60,
TokenCppKind_Tilde = 61,
TokenCppKind_Star = 62,
TokenCppKind_And = 63,
TokenCppKind_DotStar = 64,
TokenCppKind_ArrowStar = 65,
TokenCppKind_Div = 66,
TokenCppKind_Mod = 67,
TokenCppKind_LeftLeft = 68,
TokenCppKind_RightRight = 69,
TokenCppKind_Compare = 70,
TokenCppKind_Less = 71,
TokenCppKind_LessEq = 72,
TokenCppKind_Grtr = 73,
TokenCppKind_GrtrEq = 74,
TokenCppKind_EqEq = 75,
TokenCppKind_NotEq = 76,
TokenCppKind_Xor = 77,
TokenCppKind_Or = 78,
TokenCppKind_AndAnd = 79,
TokenCppKind_OrOr = 80,
TokenCppKind_Ternary = 81,
TokenCppKind_Eq = 82,
TokenCppKind_PlusEq = 83,
TokenCppKind_MinusEq = 84,
TokenCppKind_StarEq = 85,
TokenCppKind_DivEq = 86,
TokenCppKind_ModEq = 87,
TokenCppKind_LeftLeftEq = 88,
TokenCppKind_RightRightEq = 89,
TokenCppKind_Comma = 90,
TokenCppKind_PPStringify = 91,
TokenCppKind_PPConcat = 92,
TokenCppKind_Void = 93,
TokenCppKind_Bool = 94,
TokenCppKind_Char = 95,
TokenCppKind_Int = 96,
TokenCppKind_Float = 97,
TokenCppKind_Double = 98,
TokenCppKind_Long = 99,
TokenCppKind_Short = 100,
TokenCppKind_Unsigned = 101,
TokenCppKind_Signed = 102,
TokenCppKind_Const = 103,
TokenCppKind_Volatile = 104,
TokenCppKind_Asm = 105,
TokenCppKind_Break = 106,
TokenCppKind_Case = 107,
TokenCppKind_Catch = 108,
TokenCppKind_Continue = 109,
TokenCppKind_Default = 110,
TokenCppKind_Do = 111,
TokenCppKind_Else = 112,
TokenCppKind_For = 113,
TokenCppKind_Goto = 114,
TokenCppKind_If = 115,
TokenCppKind_Return = 116,
TokenCppKind_Switch = 117,
TokenCppKind_Try = 118,
TokenCppKind_While = 119,
TokenCppKind_StaticAssert = 120,
TokenCppKind_ConstCast = 121,
TokenCppKind_DynamicCast = 122,
TokenCppKind_ReinterpretCast = 123,
TokenCppKind_StaticCast = 124,
TokenCppKind_Class = 125,
TokenCppKind_Enum = 126,
TokenCppKind_Struct = 127,
TokenCppKind_Typedef = 128,
TokenCppKind_Union = 129,
TokenCppKind_Template = 130,
TokenCppKind_Typename = 131,
TokenCppKind_Friend = 132,
TokenCppKind_Namespace = 133,
TokenCppKind_Private = 134,
TokenCppKind_Protected = 135,
TokenCppKind_Public = 136,
TokenCppKind_Using = 137,
TokenCppKind_Extern = 138,
TokenCppKind_Export = 139,
TokenCppKind_Inline = 140,
TokenCppKind_Static = 141,
TokenCppKind_Virtual = 142,
TokenCppKind_AlignAs = 143,
TokenCppKind_Explicit = 144,
TokenCppKind_NoExcept = 145,
TokenCppKind_NullPtr = 146,
TokenCppKind_Operator = 147,
TokenCppKind_Register = 148,
TokenCppKind_This = 149,
TokenCppKind_ThreadLocal = 150,
TokenCppKind_SizeOf = 151,
TokenCppKind_AlignOf = 152,
TokenCppKind_DeclType = 153,
TokenCppKind_TypeID = 154,
TokenCppKind_New = 155,
TokenCppKind_Delete = 156,
TokenCppKind_Auto = 157,
TokenCppKind_Constexpr = 158,
TokenCppKind_LiteralTrue = 159,
TokenCppKind_LiteralFalse = 160,
TokenCppKind_Identifier = 161,
TokenCppKind_PPInclude = 162,
TokenCppKind_PPVersion = 163,
TokenCppKind_PPDefine = 164,
TokenCppKind_PPUndef = 165,
TokenCppKind_PPIf = 166,
TokenCppKind_PPIfDef = 167,
TokenCppKind_PPIfNDef = 168,
TokenCppKind_PPElse = 169,
TokenCppKind_PPElIf = 170,
TokenCppKind_PPEndIf = 171,
TokenCppKind_PPError = 172,
TokenCppKind_PPImport = 173,
TokenCppKind_PPUsing = 174,
TokenCppKind_PPLine = 175,
TokenCppKind_PPPragma = 176,
TokenCppKind_PPUnknown = 177,
TokenCppKind_PPDefined = 178,
TokenCppKind_COUNT = 179,
};
char *token_cpp_kind_names[] = {
"EOF",
"Whitespace",
"LexError",
"BlockComment",
"LineComment",
"Backslash",
"LiteralInteger",
"LiteralIntegerU",
"LiteralIntegerL",
"LiteralIntegerUL",
"LiteralIntegerLL",
"LiteralIntegerULL",
"LiteralIntegerHex",
"LiteralIntegerHexU",
"LiteralIntegerHexL",
"LiteralIntegerHexUL",
"LiteralIntegerHexLL",
"LiteralIntegerHexULL",
"LiteralIntegerOct",
"LiteralIntegerOctU",
"LiteralIntegerOctL",
"LiteralIntegerOctUL",
"LiteralIntegerOctLL",
"LiteralIntegerOctULL",
"LiteralFloat32",
"LiteralFloat64",
"LiteralString",
"LiteralStringWide",
"LiteralStringUTF8",
"LiteralStringUTF16",
"LiteralStringUTF32",
"LiteralStringRaw",
"LiteralStringWideRaw",
"LiteralStringUTF8Raw",
"LiteralStringUTF16Raw",
"LiteralStringUTF32Raw",
"LiteralCharacter",
"LiteralCharacterWide",
"LiteralCharacterUTF8",
"LiteralCharacterUTF16",
"LiteralCharacterUTF32",
"PPIncludeFile",
"PPErrorMessage",
"KeywordGeneric",
"BraceOp",
"BraceCl",
"ParenOp",
"BrackOp",
"ParenCl",
"BrackCl",
"Semicolon",
"Colon",
"DotDotDot",
"ColonColon",
"PlusPlus",
"MinusMinus",
"Dot",
"Arrow",
"Plus",
"Minus",
"Not",
"Tilde",
"Star",
"And",
"DotStar",
"ArrowStar",
"Div",
"Mod",
"LeftLeft",
"RightRight",
"Compare",
"Less",
"LessEq",
"Grtr",
"GrtrEq",
"EqEq",
"NotEq",
"Xor",
"Or",
"AndAnd",
"OrOr",
"Ternary",
"Eq",
"PlusEq",
"MinusEq",
"StarEq",
"DivEq",
"ModEq",
"LeftLeftEq",
"RightRightEq",
"Comma",
"PPStringify",
"PPConcat",
"Void",
"Bool",
"Char",
"Int",
"Float",
"Double",
"Long",
"Short",
"Unsigned",
"Signed",
"Const",
"Volatile",
"Asm",
"Break",
"Case",
"Catch",
"Continue",
"Default",
"Do",
"Else",
"For",
"Goto",
"If",
"Return",
"Switch",
"Try",
"While",
"StaticAssert",
"ConstCast",
"DynamicCast",
"ReinterpretCast",
"StaticCast",
"Class",
"Enum",
"Struct",
"Typedef",
"Union",
"Template",
"Typename",
"Friend",
"Namespace",
"Private",
"Protected",
"Public",
"Using",
"Extern",
"Export",
"Inline",
"Static",
"Virtual",
"AlignAs",
"Explicit",
"NoExcept",
"NullPtr",
"Operator",
"Register",
"This",
"ThreadLocal",
"SizeOf",
"AlignOf",
"DeclType",
"TypeID",
"New",
"Delete",
"Auto",
"Constexpr",
"LiteralTrue",
"LiteralFalse",
"Identifier",
"PPInclude",
"PPVersion",
"PPDefine",
"PPUndef",
"PPIf",
"PPIfDef",
"PPIfNDef",
"PPElse",
"PPElIf",
"PPEndIf",
"PPError",
"PPImport",
"PPUsing",
"PPLine",
"PPPragma",
"PPUnknown",
"PPDefined",
};