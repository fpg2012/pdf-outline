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
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>
extern "C" {
#include "pindf/pindf.h"
}
#include "utils.hpp"
#include "name_tree.hpp"
#include "page_tree.hpp"
#include "destination.hpp"

struct OutlineNode {
    std::string title;
    Destination destination;
    std::vector<OutlineNode> children;

    void from_obj(pindf_doc *doc, NameTree *name_tree, PageMap *page_map, pindf_pdf_obj *obj);

    void print(int depth = 0);

    nlohmann::json to_json() const;
    nlohmann::json to_simple_json() const;

    void from_json(const nlohmann::json &j, const PageMap *page_map);
    // pindf_pdf_obj *to_obj(pindf_doc *doc) const;
    void apply_modif(pindf_doc *doc, pindf_modif *modif, const PageMap *page_map) const;
};

struct TempOutlineNode {
    int offset = 0;
    int prev = -1, next = -1;
    int first = -1, last = -1, count = 0;
    int parent = 0;
    const OutlineNode *node = nullptr;
};

void print_outline(pindf_doc *doc);
OutlineNode *get_outline(pindf_doc *doc, NameTree *name_tree, PageMap *page_map);
pindf_pdf_obj *get_outline_obj(pindf_doc *doc);
// TempOutlineNode to_temp(const OutlineNode *outline_node, std::vector<TempOutlineNode> &nodes);