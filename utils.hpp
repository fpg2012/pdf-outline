/*
 Copyright 2026 fpg2012 (aka nth233)

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#pragma once

#include <stdexcept>
#include <vector>
extern "C" {
#include "pindf/pindf.h"
#include "pindf/container/simple_vector.h"
}

template<typename T>
std::vector<T> pindf_vector_to_std_vector(pindf_vector *vector) {
    T temp;
    std::vector<T> stdvec;
    for (size_t i = 0; i < vector->len; ++i) {
        pindf_vector_index(vector, i, &temp);
        stdvec.push_back(temp);
    }
    return stdvec;
}

pindf_pdf_obj *to_obj(int value);
pindf_pdf_obj *to_obj(double value);
pindf_pdf_obj *to_obj(float value);
pindf_pdf_obj *to_obj(pindf_pdf_obj* value);
pindf_pdf_obj *to_obj(const std::string &value);

pindf_pdf_obj *deref(pindf_doc *doc, pindf_pdf_obj *obj);

pindf_doc *parse_pdf(const char *filename);

void log_obj(pindf_pdf_obj *obj);

pindf_pdf_obj *get_catalog(pindf_doc *doc);

void utf16_to_utf8(pindf_uchar_str *str);

std::string decode_text_string(pindf_uchar_str *str);
std::string encode_text_string(const std::string& str);

std::string pindf_uchar_str_to_string(pindf_uchar_str *str);

std::string obj_to_string(pindf_pdf_obj *obj);