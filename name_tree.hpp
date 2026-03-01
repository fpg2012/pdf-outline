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