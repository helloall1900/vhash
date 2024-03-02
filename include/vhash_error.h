// Copyright (c) 2022 Leo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VHASH_ERROR_H
#define VHASH_ERROR_H

#define VERROR(e)  static_cast<int>(e)

namespace vhash {
/**
 * Errors
 */
enum class errors {
    ERR_OUT_OF_RANGE = -500,
    ERR_MK_DIR,
    ERR_OPEN_FILE,
    ERR_DECODE_IMAGE,
    ERR_READ_FILE,
    ERR_INIT_DB,
    ERR_INSERT_DB,
    ERR_DELETE_DB,
    ERR_CLEAR_DB,
    ERR_PURE_DB,
    ERR_NOT_EXISTS,
    ERR_UNKNOWN_TYPE,
    ERR_PARAM_INVALID,
    ERR_MAKE_THUMB,
};

}

#endif //VHASH_ERROR_H
