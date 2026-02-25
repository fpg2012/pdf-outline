#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

extern "C" {
#include "pindf/pindf.h"
}

struct NameTree {
    std::unordered_map<std::string, pindf_pdf_obj*> names_to_page;
    /// borrow from underlying document
    pindf_pdf_obj *name_tree_obj;

    void from_obj(pindf_doc *doc, pindf_pdf_obj *obj);
    void print();
    nlohmann::json to_json(pindf_doc *doc);
    pindf_pdf_obj* get_dest(const std::string& name);
};

NameTree *get_name_tree(pindf_doc *doc);