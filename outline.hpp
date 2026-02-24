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