#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include "name_tree.hpp"
#include "page_tree.hpp"
extern "C" {
#include "pindf/pindf.h"
}

struct Destination {
    int page;
    pindf_pdf_obj *page_obj;
    pindf_pdf_obj *dest_obj;

    std::string name;
    std::vector<pindf_pdf_obj*> dest_arr;

    Destination() : page(-1), page_obj(nullptr), dest_obj(nullptr) {}

    void from_obj(pindf_doc *doc, NameTree *name_tree, PageMap *page_map, pindf_pdf_obj *dest);
    nlohmann::json to_json() const;
    void from_json(const nlohmann::json &j, const PageMap *page_map);

    pindf_pdf_obj *to_obj(pindf_doc* doc, const PageMap *page_map) const;

    void init_default(const PageMap *page_map, int page_no = -1);
};
