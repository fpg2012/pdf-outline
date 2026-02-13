#pragma once

#include <string>
#include <vector>
#include <iostream>
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
};

void print_outline(pindf_doc *doc);
OutlineNode *get_outline(pindf_doc *doc, NameTree *name_tree, PageMap *page_map);