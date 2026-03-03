# !/usr/bin/python
'''
Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
SPDX-License-Identifier: BSD-3-Clause
https://spdx.org/licenses
'''

import os
import sys
import re
import time
import argparse
import shutil

# MACROs
AUTOGEN_OK   = 0
AUTOGEN_FAIL = -1


# global variables
module = ""
input_file  = None
output_file = None
input_file_path  = ""
output_file_path = ""
tmp_output_file  = ""
input_file_eof   = 0
input_file_line  = ""
manual_cli_tbl   = []
no_cli_entry_tag = "__NO_CLI_ENTRY__"


api_desc = {
    "prefix": "clsapi",
    "action": "",
    "module": "",
    "param":  "",
    "name":   "",
    "cmt_desc":  [{"name": "", "flags": 0}],
    "cmt_srcs":  [""],
    "arg_desc":  [{"name": "", "attr": 0, "data_type": "", "arr_len": ""}],
    "decl_srcs": [""]
}

def api_desc_init():
    api_desc["action"] = ""
    api_desc["module"] = ""
    api_desc["param"]  = ""
    api_desc["name"]   = ""
    api_desc["cmt_desc"] = []
    api_desc["cmt_srcs"] = []
    api_desc["arg_desc"] = []
    api_desc["decl_srcs"] = []
    return

# List to contain cli entries generated per input file
cli_entry_table = [
    ""
]


'''
Generic cli handler matching are done by C-Call API arguments flags.
Each argument is assigned to following 32bits flags. In CLS-API, at most
4 arguments are supported. So CLS-API C-Call API arguments (at most 4)
are 128bits.
128 bits = arg1 | arg2 | arg3 | arg4
per arg bit definition:
bit00~04: (5bits) API data types
bit05~09: flags
bit10~12: reserved
bit13~16: (4bits) predefined array len(0:8, 1:16, 2:32, 3:64, 4:128, 5:256, 6:512, 7:1024, 8:2048, 9:4096, 10:8192)
'''
DATA_TYPE_MASK      = 0b11111

DATA_TYPE_VOID      = 0
DATA_TYPE_CHAR      = 1 # real char
DATA_TYPE_BOOL      = 2
DATA_TYPE_INT8      = 3 # 8bits signed integer
DATA_TYPE_SHORT     = 4
DATA_TYPE_INT       = 5
DATA_TYPE_ENUM      = 6
DATA_TYPE_LONG      = 7
DATA_TYPE_UCHAR     = 8
DATA_TYPE_USHORT    = 9
DATA_TYPE_UINT      = 10
DATA_TYPE_ULONG     = 11
DATA_TYPE_FLOAT     = 12
DATA_TYPE_DOUBLE    = 13
DATA_TYPE_STRUCT    = 14
DATA_TYPE_UNION     = 15
DATA_TYPE_MAX       = 16 # MUST <=31
DATA_TYPE_UNKNOWN   = DATA_TYPE_MAX

ARG_FLAGS_IN        = (1 << 5)
ARG_FLAGS_OUT       = (1 << 6)
ARG_FLAGS_ARRAY     = (1 << 7)
ARG_FLAGS_PTR       = (1 << 8)
ARG_FLAGS_CONST     = (1 << 9)
ARR_LEN_BIT_START   = 13
ARR_LEN_MASK        = 0b1111
ARG_BITS            = 32


# predefined array length definition
ARRAY_LEN_8         = 0
ARRAY_LEN_16        = 1
ARRAY_LEN_32        = 2
ARRAY_LEN_64        = 3
ARRAY_LEN_128       = 4
ARRAY_LEN_256       = 5
ARRAY_LEN_512       = 6
ARRAY_LEN_1024      = 7
ARRAY_LEN_2048      = 8
ARRAY_LEN_4096      = 9
ARRAY_LEN_8192      = 10
array_len_list = [8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]

# List to mapping data types in string to int
# Why not regular expression? To avoid typedefed data types are recgonised
# wrongly as basic data types.
data_type_list_table = [
    ["void"], # DATA_TYPE_VOID
    ["char"], # DATA_TYPE_CHAR
    ["bool"], # DATA_TYPE_BOOL
    ["int8", "int8_t", "s8", "s8_t", "sint8", "sint8_t"], # DATA_TYPE_INT8
    ["short", "short int", "int16", "int16_t", "s16", "s16_t", "sint16", "sint16_t"], # DATA_TYPE_SHORT
    ["int", "int32", "int32_t", "s32", "s32_t", "sint32", "sint32_t"], # DATA_TYPE_INT
    ["enum"], # DATA_TYPE_ENUM
    ["long", "long int", "int64", "int64_t", "s64", "s64_t", "sint64", "sint64_t"], # DATA_TYPE_LONG
    ["unsigned char", "uint8", "uint8_t", "u8", "u8_t", "uchar", "uchar_t"], # DATA_TYPE_UCHAR
    ["unsigned short", "unsigned short int", "uint16", "uint16_t", "u16", "u16_t", "ushort", "ushort_t"], # DATA_TYPE_USHORT
    ["unsigned int", "unsigned int", "uint32", "uint32_t", "u32", "u32_t", "uint", "uint_t"], # DATA_TYPE_UINT
    ["unsigned long", "unsigned long int", "uint64", "uint64_t", "u64", "u64_t", "ulong", "ulong_t"], # DATA_TYPE_ULONG
    ["float"], # DATA_TYPE_FLOAT
    ["double"], # DATA_TYPE_DOUBLE
    ["struct"], # DATA_TYPE_STRUCT
    ["union"], # DATA_TYPE_UNION
    ["unknown data types"], # DATA_TYPE_UNKNOWN
]


# dictionary of (args_attrs, str_generic_handler_ptr)
attr_clsapi_get_out_char1024              = ((ARRAY_LEN_1024<<ARR_LEN_BIT_START) | ARG_FLAGS_ARRAY | ARG_FLAGS_OUT | DATA_TYPE_CHAR)
attr_clsapi_get_in_charptr_out_char1024   = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | ((ARRAY_LEN_1024<<ARR_LEN_BIT_START)|ARG_FLAGS_ARRAY|ARG_FLAGS_OUT|DATA_TYPE_CHAR))
attr_clsapi_get_in_charptr_out_u32ptr     = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_PTR|ARG_FLAGS_OUT|DATA_TYPE_UINT))
attr_clsapi_get_in_charptr_out_u8ptr      = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_PTR|ARG_FLAGS_OUT|DATA_TYPE_UCHAR))
attr_clsapi_get_in_charptr_out_s32ptr     = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_PTR|ARG_FLAGS_OUT|DATA_TYPE_INT))
attr_clsapi_get_in_charptr_s32_out_u32ptr = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<(ARG_BITS*2)) | ((ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_INT)<<ARG_BITS) | (ARG_FLAGS_PTR|ARG_FLAGS_OUT|DATA_TYPE_UINT))

args_attrs_generic_api_type_get = [
    {"args_attrs": attr_clsapi_get_out_char1024,              "clsapi_api_type": "clsapi_get_out_char1024"},
    {"args_attrs": attr_clsapi_get_in_charptr_out_char1024,   "clsapi_api_type": "clsapi_get_in_charptr_out_char1024"},
    {"args_attrs": attr_clsapi_get_in_charptr_out_u32ptr,     "clsapi_api_type": "clsapi_get_in_charptr_out_u32ptr"},
    {"args_attrs": attr_clsapi_get_in_charptr_out_u8ptr,      "clsapi_api_type": "clsapi_get_in_charptr_out_u8ptr"},
    {"args_attrs": attr_clsapi_get_in_charptr_out_s32ptr,     "clsapi_api_type": "clsapi_get_in_charptr_out_s32ptr"},
    {"args_attrs": attr_clsapi_get_in_charptr_s32_out_u32ptr, "clsapi_api_type": "clsapi_get_in_charptr_s32_out_u32ptr"},
]


attr_clsapi_set_charptr         = (ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)
attr_clsapi_set_charptr_charptr = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR))
attr_clsapi_set_charptr_s8      = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_INT8))
attr_clsapi_set_charptr_s32     = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_INT))
attr_clsapi_set_charptr_u8      = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_UCHAR))
attr_clsapi_set_charptr_u32     = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<ARG_BITS) | (ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_UINT))
attr_clsapi_set_charptr_s32_s32 = (((ARG_FLAGS_CONST|ARG_FLAGS_PTR|ARG_FLAGS_IN|DATA_TYPE_CHAR)<<(ARG_BITS*2)) | ((ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_INT)<<ARG_BITS) | (ARG_FLAGS_CONST|ARG_FLAGS_IN|DATA_TYPE_INT))

args_attrs_generic_api_type_set = [
    {"args_attrs": attr_clsapi_set_charptr,         "clsapi_api_type": "clsapi_set_charptr"},
    {"args_attrs": attr_clsapi_set_charptr_charptr, "clsapi_api_type": "clsapi_set_charptr_charptr"},
    {"args_attrs": attr_clsapi_set_charptr_s8,      "clsapi_api_type": "clsapi_set_charptr_s8"},
    {"args_attrs": attr_clsapi_set_charptr_s32,     "clsapi_api_type": "clsapi_set_charptr_s32"},
    {"args_attrs": attr_clsapi_set_charptr_u8,      "clsapi_api_type": "clsapi_set_charptr_u8"},
    {"args_attrs": attr_clsapi_set_charptr_u32,     "clsapi_api_type": "clsapi_set_charptr_u32"},
    {"args_attrs": attr_clsapi_set_charptr_s32_s32, "clsapi_api_type": "clsapi_set_charptr_s32_s32"},
]


# Functions

def set_input_file_eof():
    global input_file_eof
    input_file_eof = 1
    return AUTOGEN_FAIL


def parse_manual_cli_table(file_clsapi_cli_xx_c):
    global manual_cli_tbl
    found_manual_cli_tbl = 0
    found_manual_cli_tbl_end = 0
    manual_cli_tbl_def_line = "static struct clsapi_cli_entry clsapi_cli_entry_" + module + "_manual\[\] = \{"
    with open(file_clsapi_cli_xx_c, mode='r') as manual_cli_file:
        for line in manual_cli_file:
            if found_manual_cli_tbl == 0:
                match_manual_cli_tbl = re.match(manual_cli_tbl_def_line, line)
                if match_manual_cli_tbl:
                    found_manual_cli_tbl = 1
            else:
                match_cli_tbl_end = re.match(r"\};", line)
                if match_cli_tbl_end:
                    found_manual_cli_tbl_end = 1
                    break
                manual_cli_tbl.append(line)
    if found_manual_cli_tbl and found_manual_cli_tbl_end:
        return AUTOGEN_OK
    else:
        raise RuntimeError("No manual cli entry table found.\nclsapi_cli_entry_{0}_manual[]\n".format(module))
        return AUTOGEN_FAIL


# Process input args
# usage:   autogen_clsapi_cli.py <input src file> <ouput src file>
# Example: autogen_clsapi_cli.py wifi/clsapi_wifi.h wifi/autogened_cli_wifi.h
def process_args_get_file_obj():
    global module, input_file, output_file, input_file_path
    global output_file_path, tmp_output_file
    parser = argparse.ArgumentParser(
        description="Automatic generate CLS-API cli entries and handlers per clsapi_xx.h"
    )
    parser.add_argument("input_file_path")
    parser.add_argument("output_file_path")
    args = parser.parse_args()

    input_file_path = args.input_file_path
    output_file_path = args.output_file_path

    # Validate and open input file
    if not os.path.exists(input_file_path):
        print("The input file '" + input_file_path + "' is not exist.")
        return AUTOGEN_FAIL

    # input file name in format "[xxxx/]clsapi_<module>.h"
    module_match = re.search(r"clsapi_(.*)\.h$", input_file_path)
    if module_match is None:
        print("File '" + input_file_path + "' is not clsapi_xx.h")
        return AUTOGEN_FAIL
    module = module_match.group(1)
    # e.g. wifi/clsapi_wifi.h ==> wifi/cli_wifi.c
    file_clsapi_cli_xx_c = input_file_path
    file_clsapi_cli_xx_c = file_clsapi_cli_xx_c.replace("clsapi_", "cli_")
    file_clsapi_cli_xx_c = file_clsapi_cli_xx_c.replace(".h", ".c")
    if parse_manual_cli_table(file_clsapi_cli_xx_c) == AUTOGEN_FAIL:
        return AUTOGEN_FAIL

    input_file = open(input_file_path, mode='r')

    # Open output file for write
    tmp_output_file = "/tmp/.autogen_clsapi_cli_" + str(time.time()) + ".h"
    output_file = open(tmp_output_file, mode='w')
    return AUTOGEN_OK


# skip code lines of API comments and declaration from current to end of this API declaration
def skip_api_cmt_decl():
    cmd_end_skipped = 0
    for line in input_file:
        # search comment ending "*/"
        cmt_end_match = re.search(r'\*/', line)
        if cmt_end_match or cmd_end_skipped:
            cmd_end_skipped = 1
            # search declaration ending ";"
            api_decl_end = re.search(r';', line)
            if api_decl_end:
                return AUTOGEN_OK
    else:
        return set_input_file_eof()


# translate dir string to flag bits
# \param dir [in] direction of the arg, possible values are: "in", "out", "in,out"
# Return value: arg direction flags
#    ARG_FLAGS_IN
#    ARG_FLAGS_OUT
#    ARG_FLAGS_IN | ARG_FLAGS_OUT
def param_dir_to_flag(dir):
    dir_flag = 0;
    if "In" in dir:
        dir_flag |= ARG_FLAGS_IN
    if "Out" in dir:
        dir_flag |= ARG_FLAGS_OUT
    return dir_flag


# Parse API comments and extract arg name and direction (in, out, in_out)
# Constrains of API comments:
#   1. [in] args first, then [out] and [in,out]
#   2. Argument sequence in comment is same to it's in API declaration
#   3. 'param' and 'brief' MUST be NOT in same line
# Return value:
#   AUTOGEN_OK:   OK, have API comments
#   AUTOGEN_FAIL: Error or no API comments
def parse_api_comments():
    line = ""
    for line in input_file:
        # search no cli entry tag, and skip this API comments and declaration
        no_cli = re.search(no_cli_entry_tag, line)
        if no_cli:
            # no need to gen cli entry and handler
            # skip this API's comments and declaration lines
            skip_api_cmt_decl()
            return AUTOGEN_FAIL

        # Search brief line, it's beginning of api-comment-declare pair
        brief_match = re.search(r'[@\\]brief', line)
        if brief_match:
            api_desc["cmt_srcs"].append(line)
            break
    else:
        # EOF
        return set_input_file_eof()

    # Parse params in comments
    for line in input_file:
        api_desc["cmt_srcs"].append(line)
        param = re.search(r'[@\\]param\s+(\w+)\s+\[(In|Out|In,Out)\]\s+(.+)', line)
        if param:
            # param comment format: "\param <param name> [in|out|in,out] <param descriptions>"
            name = param.group(1)
            dir_flag = param_dir_to_flag(param.group(2))
            api_desc["cmt_desc"].append({"name": name, "flags": dir_flag})
        cmt_end_match = re.search(r'\*/', line)
        if cmt_end_match:
            break
    else:
        # EOF
        return set_input_file_eof()

    return AUTOGEN_OK


# Convert data type in string (arg["data_type"]) to attribution in integer
# Constrains:
#   1. typedefed data types are NOT supported, except famious one, e.g. uint32_t.
# Return value:
#   >= 0: converted data types
#   < 0:  unknown date types
def str_data_type_to_attr(arg):
    str_data_type = arg["data_type"]
    data_type_idx = 0

    # spatial care for string_N, convert "string_N abc" to "char abc[N+1]"
    str_match = re.search(r'string_(\d+)', str_data_type)
    if str_match:
        arg["arr_len"] = str_match.group(1)
        int_arr_len = int(arg["arr_len"])
        try:
            arr_len_idx = array_len_list.index(int_arr_len)
        except ValueError:
            raise RuntimeError("Unknown data type'" + str_data_type + "'.")
        else:
            arg["data_type"] = "char"
            arg["attr"] |= ARG_FLAGS_ARRAY
            arg["attr"] |= (arr_len_idx << ARR_LEN_BIT_START)
        arg["attr"] |= DATA_TYPE_CHAR
        return arg["attr"]

    # search table to match data type, index = int_data_type
    for type_list in data_type_list_table:
        if str_data_type in type_list:
            break
        data_type_idx += 1
    arg["attr"] |= data_type_idx
    return arg["attr"]


# handle data_type which may contains '*'. and extract pure data_type
# E.g. data_type = "int *" ==>
#      data_type = "int" and
#      arg_desc["attr"] |= ARG_FLAGS_PTR
# \param data_type [in] data type which may contains '*'.
# \param arg_desc [out] arg_desc to contain converted pure data_type and flags
def handle_ptr_in_data_type(data_type, arg_desc):
    data_type = data_type.strip()
    # data_type may contains '*'
    ptr_pat = re.compile('\*')
    data_type_sub = ptr_pat.subn("", data_type)
    ptr_level = data_type_sub[1]
    if ptr_level > 1:
        raise RuntimeError("Multi-level pointer is NOT supported!!")
    elif ptr_level == 1:
        arg_desc["attr"] |= ARG_FLAGS_PTR
    arg_desc["data_type"] = data_type_sub[0].strip()
    return


# Parse API args in one line string to api_desc["arg_desc"]
# \param str_args [in] args in one line string
# Constraints for arg in API declaration:
#  1. arg name MUST be provided.
#  2. struct, enum and function pointer data types are NOT supported.
#  3. Only modifier "const" is supported.
# return:
#    AUTOGEN_OK :  OK
#    AUTOGEN_FAIL: Errors
def parse_api_args_to_desc(str_args):
    # Check if there are unsupported features
    err_msg = ""
    invalid_match = re.findall(r'(\(|struct|enum|union)', str_args)
    if invalid_match:
        err_msg = "Unsupported argument types:\n"
        for invalid_feat in invalid_match:
            if invalid_feat == "(":
                err_msg += "function pointer\n"
            else:
                err_msg += invalid_feat + "\n"
        raise RuntimeError(err_msg)

    if str_args == "void" or str_args == "":
        # void or no arg, do nothing
        return AUTOGEN_OK

    # split args to arg list
    arg_idx = 0
    arg_list = str_args.split(',')
    for arg in arg_list:
        arg = arg.strip()
        # arg = <data type> + <arg name>, get <arg name>
        arg_name = re.match(r'(const)?\s*(.*)\s*\b(\w+)\b(\[(\w+)*\])?$', arg)
        if arg_name is None:
            # No arg name, invalid, return
            raise RuntimeError("No argument name", arg)

        arg_desc = {"name": "", "data_type": "", "attr": 0, "arr_len": ""}
        arg_desc["attr"] = api_desc["cmt_desc"][arg_idx]["flags"]
        if (arg_name.group(1)): # const
            arg_desc["attr"] |= ARG_FLAGS_CONST
        if (arg_name.group(2)): # data_type may contains '*'
            handle_ptr_in_data_type(arg_name.group(2), arg_desc)
            str_data_type_to_attr(arg_desc)
        else:
            raise RuntimeError("No data type.", arg)
        if (arg_name.group(3)): # arg name
            arg_desc["name"] = arg_name.group(3)
        else:
            raise RuntimeError("No arg name.", arg)
        if (arg_name.group(4)): # array or not
            arg_desc["attr"] |= ARG_FLAGS_ARRAY
        if (arg_name.group(5)): # array len
            arg_desc["arr_len"] = arg_name.group(5)
            if arg_desc["arr_len"] == "ETH_ALEN":
                arg_desc["arr_len"] = "6"

        api_desc["arg_desc"].append(arg_desc)
        arg_idx += 1
    return AUTOGEN_OK


''' Parse API declaration and extract func and arg infos
API declaration constraint:
  1. No comments in API declaration lines
  2. func return value "int", func name and '(' MUST be in same line
Code flow:
  1. Find API declaration prefix "int clsapi", this is 1st line of API declaration.
  2. Find API declaration end symbol ';', this is last line of API declaration.
  3. Concatenate lines from 1st to last to one line
Return values:
    AUTOGEN_OK:   OK
    AUTOGEN_FAIL: Errors
'''
def parse_api_declaration():
    api_decl_1st  = re.compile(r'.*int\s+clsapi_([a-z]+)_([a-z]+)_(\w+)\s*\(')
    api_decl_pat  = re.compile(r'.*int\s+clsapi_([a-z]+)_([a-z]+)_(\w+)\s*\((.*?)\)')
    api_decl_end  = re.compile(r';')
    api_decl_cat  = ""
    line = ""
    # Search "int clsapi_" prefix, it's beginning of API declaration
    for line in input_file:
        line = line.strip()
        api_decl_1st_m = api_decl_1st.match(line)
        if api_decl_1st_m:
            api_desc["decl_srcs"].append(line)
            break
    else:
        set_input_file_eof()
        raise RuntimeError("no API declaration found for comments!!")

    api_decl_cat = line
    last_desc_line = api_decl_end.search(line)
    if last_desc_line is None:
        # API declaration spans multiple lines.
        # Why not validate then loop?
        #  1. do-while loop is NOT supported in Python
        #  2. Python says "Mixing iteration and read methods would lose data"
        for line in input_file:
            api_desc["decl_srcs"].append(line)
            # Concatenate API declaration lines to one line
            line = line.strip()
            api_decl_cat += line
            last_desc_line = api_decl_end.search(line)
            if last_desc_line:
                break
        else:
            # EOF
            return set_input_file_eof()
    # API declaration is parsed and concantenated to one line

    # Get action, module, param and arg_block
    api_decl_m = api_decl_pat.match(api_decl_cat)
    if api_decl_m is None:
        raise RuntimeError("Invalid API declaration:\n" + api_decl_cat)

    api_desc["module"] = api_decl_m.group(1)
    api_desc["action"] = api_decl_m.group(2)
    api_desc["param"]  = api_decl_m.group(3)
    api_desc["name"]   = "{0}_{1}_{2}_{3}".format(api_desc["prefix"], api_desc["module"], api_desc["action"], api_desc["param"])
    if is_api_in_manual_cli():
        return AUTOGEN_FAIL
    return parse_api_args_to_desc(api_decl_m.group(4).strip())


# Validation API descriptor:
# 1. arg mapping between comments and declaration
# 2. Filter un-supported features
# Return values:
#   OK: return
#   Errors: raise exception
def api_desc_validation():
    arg_idx = 0;
    cmt_arg_list = api_desc["cmt_desc"]
    decl_arg_list = api_desc["arg_desc"]

    # arg mapping between comments and declaration
    if len(cmt_arg_list) != len(decl_arg_list):
        raise RuntimeError("Argument number is mismatch in comments and declaration!!")
    for decl_arg in decl_arg_list:
        attr = decl_arg["attr"]
        name = decl_arg["name"]
        cmt_arg = cmt_arg_list[arg_idx]
        if cmt_arg["name"] != name:
            raise RuntimeError("Argument name is mismatch in comment '{0}' and declaration '{1}' for ".format(cmt_arg["name"], name))
        if attr & ARG_FLAGS_IN and attr & ARG_FLAGS_OUT:
            # [in,out] is not supported yet
            raise RuntimeError("Argument '{0}' with [in,out] is not supported yet!!".format(name))
        if attr & ARG_FLAGS_IN and not(attr & ARG_FLAGS_CONST):
            # input arg should with 'const'
            raise RuntimeError("Input argument '{0}' MUST be 'const'!!".format(name))
        if attr & ARG_FLAGS_OUT:
            if attr & ARG_FLAGS_CONST:
                # Output arg should NOT be 'const'
                raise RuntimeError("Output argument '{0}' MUST NOT be 'const'!!".format(name))
            if not(attr & ARG_FLAGS_PTR) and not(attr & ARG_FLAGS_ARRAY):
                # Output arg should be 'pointer' or 'array'
                raise RuntimeError("Output argument '{0}' MUST be pointer or array!!".format(name))
            if attr & ARG_FLAGS_ARRAY and (attr & DATA_TYPE_MASK != DATA_TYPE_CHAR):
                # un-supported data type
                raise RuntimeError("Non-char array, '{0}', as output argument is not supported yet!!".format(name))
        arg_idx += 1
    return


def get_cast_func_name(attr):
    data_type = attr & DATA_TYPE_MASK
    if data_type == DATA_TYPE_CHAR and ((attr & ARG_FLAGS_PTR) or (attr & ARG_FLAGS_ARRAY)):
        # char* and char[]
        return ""
    elif data_type >= DATA_TYPE_BOOL and data_type <=  DATA_TYPE_ULONG:
        return "atol"
    elif data_type == DATA_TYPE_FLOAT or data_type == DATA_TYPE_DOUBLE:
        return "atof"
    return ""


def gen_cli_handler_by_api_desc(hdl_func, api_name):
    is_get_hdl = 0
    in_arg_idx = 0
    code_lines = []
    arg_list_in_call = []
    code = "static int " + hdl_func + '''(struct clsapi_cli_output *output, struct clsapi_cli_entry *cli_entry, int argc, char **argv)
{
\tint ret = CLSAPI_OK;
'''
    code_lines.append(code)

    # gen code of variable declaration and assignment per arg_desc
    for arg in api_desc["arg_desc"]:
        arg_attr = arg["attr"]
        const = ""
        ptr = ""
        code = ""
        data_type = arg_attr & DATA_TYPE_MASK
        # gen input args
        if arg_attr & ARG_FLAGS_IN:
            arg_list_in_call.append(arg["name"])
            if arg_attr & ARG_FLAGS_CONST: const = "const"
            if arg_attr & (ARG_FLAGS_PTR | ARG_FLAGS_ARRAY):
                ptr = "*"
            # Only 2 arrays are supported, "char[]"/string and "uint8_t [6]"/mac_addr
            if arg_attr & ARG_FLAGS_ARRAY and arg["arr_len"] == "6" and data_type == DATA_TYPE_UCHAR:
                # input arg is mac address
                code += "\tuint8_t " + arg["name"] + "[ETH_ALEN] = {0};\n\n"
                code += "\tret = mac_aton(argv[{0}], {1});\n".format(in_arg_idx, arg["name"])
                code += "\tif (ret) return -CLSAPI_ERR_INVALID_PARAM;\n"
            else:
                cast_func = get_cast_func_name(arg_attr)
                # in variable code format: 0-[const] 1-<data_type> 2-[*] 3-<variable name> = 4-[cast_func] (argv[5-<arg_idx>]);
                code += "\t{0} {1} {2}{3} = {4}(argv[{5}]);\n".format(const, arg["data_type"], ptr, arg["name"], cast_func, in_arg_idx)
            in_arg_idx += 1

        # gen output args declaration
        if arg_attr & ARG_FLAGS_OUT:
            # 3 types of out arg: int *a, char *a and int a[] / char a[]
            is_get_hdl = 1
            get_addr = "&"
            if (data_type == DATA_TYPE_CHAR) and (arg_attr & ARG_FLAGS_ARRAY):
                # char array "string_N"
                get_addr = ""
                code = "\t{0} {1}[{2}]".format(arg["data_type"], arg["name"], int(arg["arr_len"]) + 1)
                code += " = {0};\n"
            elif data_type == DATA_TYPE_CHAR and (arg_attr & ARG_FLAGS_PTR):
                # char *
                get_addr = ""
                code = "\tchar " + arg["name"] + "[1024] = {0};\n"
            else:
                code = "\t" + arg["data_type"] + " " + arg["name"] + " = 0;\n"
            arg_list_in_call.append(get_addr + arg["name"])
        code_lines.append(code)

    # gen code of C-Call API call
    code = "\n\tret = " + api_name + "(" + ", ".join(arg_list_in_call) + ");\n"
    code_lines.append(code)

    # gen code of cli_report
    if is_get_hdl == 0:
        # handler for set APIs: just report complete
        code = "\n\treturn clsapi_cli_report_complete(ret, output);\n"
    else:
        # handler for get APIs: report got value for [out] args
        for arg in api_desc["arg_desc"]:
            attr = arg["attr"]
            if attr & ARG_FLAGS_OUT:
                data_type = attr & DATA_TYPE_MASK
                if data_type == DATA_TYPE_CHAR and (attr & (ARG_FLAGS_ARRAY|ARG_FLAGS_PTR)):
                    code = "\n\treturn clsapi_cli_report_str_value(ret, output, " + arg["name"] + ");\n"
                elif data_type >= DATA_TYPE_BOOL and data_type <= DATA_TYPE_ENUM:
                    code = "\n\treturn clsapi_cli_report_int_value(ret, output, " + arg["name"] + ");\n"
                elif data_type == DATA_TYPE_LONG:
                    code = "\n\treturn clsapi_cli_report_long_value(ret, output, " + arg["name"] + ");\n"
                elif data_type >= DATA_TYPE_UCHAR and data_type <= DATA_TYPE_UINT:
                    code = "\n\treturn clsapi_cli_report_uint_value(ret, output, " + arg["name"] + ");\n"
                elif data_type == DATA_TYPE_ULONG:
                    code = "\n\treturn clsapi_cli_report_ulong_value(ret, output, " + arg["name"] + ");\n"
                #elif data_type >= DATA_TYPE_FLOAT and data_type <= DATA_TYPE_DOUBLE:
                #    code = "\n\treturn clsapi_cli_report_float_value(ret, output, " + arg["name"] + ");\n"
                else:
                    # un-supported data type
                    raise RuntimeError("Un-supported data type:" + arg["data_type"])
            else:
                continue
    code_lines.append(code)

    # gen code of function ending
    code_lines.append("}\n\n")

    # write code lines to output file
    for line in code_lines:
        output_file.write(line)

    return


# convert api_desc["arg_desc"] to 64bits attribution
def get_args_attrs():
    arg_idx = 0
    args_attrs = 0;
    for arg in api_desc["arg_desc"]:
        arg_attr = arg["attr"]
        args_attrs = (args_attrs << ARG_BITS) | arg_attr
        arg_idx += 1

    return args_attrs


def find_generic_cli_hdl(args_attrs, attr_type_list):
    # matching generic handler table by arg attributes
    for hdl in attr_type_list:
        if args_attrs == hdl["args_attrs"]:
            return hdl["clsapi_api_type"]

    # No generic handler
    return None

# gen cli entry for this API
# if min_arg != max_arg, handle it manually.
def gen_cli_entry_by_api_desc():
    arg_n = 0
    usage = ""

    cmd = api_desc["action"] + " " + api_desc["param"]
    for arg in api_desc["arg_desc"]:
        if arg["attr"] & ARG_FLAGS_IN:
            if arg_n == 0:
                usage = "<" + arg["name"] + ">"
            else:
                usage += " <" + arg["name"] + ">"
            arg_n += 1
    # {0-cmd, 1-min_arg, 2-max_arg, 3-usage, 4-cli_handler[, 5-C_API(hdl_func_ptr, api_func)]},
    cli_entry = "\"{0}\", {1}, {2}, \"{3}\"".format(cmd, arg_n, arg_n, usage)

    # get cli generic handler or specific handler
    args_attrs = get_args_attrs()
    set_api_type = None
    api_name = api_desc["name"]
    get_api_type = find_generic_cli_hdl(args_attrs, args_attrs_generic_api_type_get)
    if get_api_type is None:
        set_api_type = find_generic_cli_hdl(args_attrs, args_attrs_generic_api_type_set)
    if get_api_type:
        cli_entry = cli_entry + ", cli_generic_get, C_API(" + get_api_type + ", " + api_name + ")"
    elif set_api_type:
        cli_entry = cli_entry + ", cli_generic_set, C_API(" + set_api_type + ", " + api_name + ")"
    else:
        # not in generic cli handler
        hdl_func = "cli_" + api_desc["action"] + "_" + api_desc["param"]
        gen_cli_handler_by_api_desc(hdl_func, api_name)
        cli_entry = cli_entry + ", " + hdl_func

    cli_entry = "\t{" + cli_entry + "},\n"
    cli_entry_table.append(cli_entry)
    return


def is_api_in_manual_cli():
    pat_api_cmd = "\t\{\"" + api_desc["action"] + " " + api_desc["param"] + "\""
    for manual_cli_line in manual_cli_tbl:
        match_api_cmd = re.match(pat_api_cmd, manual_cli_line)
        if match_api_cmd:
            return 1
    return 0


def parse_input_generate_cli_entries():
    while input_file_eof == 0:
        api_desc_init()
        # Parse API comments
        if parse_api_comments() == AUTOGEN_OK:
            if parse_api_declaration() == AUTOGEN_OK:
                api_desc_validation()
                gen_cli_entry_by_api_desc()


def autogen_clsapi_cli_xx_h():
    global output_file
    filename = output_file_path
    macro_name = "_" + filename.replace('.', '_').replace('-', '_').upper()
    output_file.write(
'''/* Automatically generated file; DO NOT EDIT. */
/*
 * Copyright (C) 2024 Clourney Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 * https://spdx.org/licenses
 *
 */

''')
    output_file.write("#ifndef {0}\n#define {0}\n\n#include <stdio.h>\n#include <stdlib.h>\n\n".format(macro_name))
    parse_input_generate_cli_entries()
    # Write cli entry to output file
    output_file.write("\n")
    output_file.write("struct clsapi_cli_entry clsapi_cli_entry_" + module + "_auto[] = {\n")
    for entry in cli_entry_table:
        output_file.write(entry)
    output_file.write(
'''
\t{}
};

''')
    output_file.write("#endif /* {0} */".format(macro_name))



def main():
    try:
        ret = process_args_get_file_obj()
        if ret == AUTOGEN_FAIL:
            sys.exit()

        ret = autogen_clsapi_cli_xx_h()
        if ret == AUTOGEN_FAIL:
            print("Error in autogen_clsapi_cli_xx_h()! Exit")
            sys.exit()
    except Exception as err:
        print("\nError: " + str(err))
        for cmt_src in api_desc["cmt_srcs"]:
            print(cmt_src.rstrip())
        for decl_src in api_desc["decl_srcs"]:
            print(decl_src.rstrip())
        print("\n")
        sys.exit()

    global output_file, tmp_output_file, output_file_path
    output_file.close()
    shutil.move(tmp_output_file, output_file_path)
    print(output_file_path + " is generated successfully.")

if __name__ == "__main__":
    main()

