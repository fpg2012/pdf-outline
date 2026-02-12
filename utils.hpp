#pragma once

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

pindf_pdf_obj *deref(pindf_doc *doc, pindf_pdf_obj *obj);

pindf_doc *parse_pdf(const char *filename);

void log_obj(pindf_pdf_obj *obj);

pindf_pdf_obj *get_catalog(pindf_doc *doc);

void utf16_to_utf8(pindf_uchar_str *str);

std::string decode_text_string(pindf_uchar_str *str);

std::string pindf_uchar_str_to_string(pindf_uchar_str *str);