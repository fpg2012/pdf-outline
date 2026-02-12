#pragma once
#include <vector>
#include <unordered_map>

extern "C" {
#include "pindf/pindf.h"
}
#include "utils.hpp"

struct PageMap {
    std::vector<int> obj_nums;
    std::unordered_map<int, int> obj_num_to_page_index;

    void from_obj(pindf_doc* doc, pindf_pdf_obj* obj);
    int get_index(int obj_num);
};

PageMap *get_pages(pindf_doc *doc);
