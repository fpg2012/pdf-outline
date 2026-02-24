#include "name_tree.hpp"
#include <iostream>
#include "utils.hpp"

void NameTree::from_obj(pindf_doc *doc, pindf_pdf_obj *obj) {
    name_tree_obj = obj;
    obj = deref(doc, obj);
    // debug
    // std::cout << "name tree loading: " << std::endl;
    // log_obj(obj);
    // std::cout << std::endl;

    if (obj == nullptr) {
        std::cerr << "Name tree object is null" << std::endl;
        return;
    }

    pindf_pdf_obj *kids = pindf_dict_getvalue2(&obj->content.dict, "/Kids");
    kids = deref(doc, kids);

    // not a leaf
    if (kids != nullptr) {
        if (kids->obj_type != PINDF_PDF_ARRAY) {
            std::cerr << "Kids is not an array" << std::endl;
            return;
        }
        std::vector<pindf_pdf_obj*> vec = pindf_vector_to_std_vector<pindf_pdf_obj*>(kids->content.array);
        for (pindf_pdf_obj *kid : vec) {
            from_obj(doc, deref(doc, kid));
        }
        return;
    }

    // leaf
    pindf_pdf_obj *names = pindf_dict_getvalue2(&obj->content.dict, "/Names");
    names = deref(doc, names);
    if (names == nullptr) {
        std::cerr << "Names is null" << std::endl;
        return;
    }
    std::vector<pindf_pdf_obj*> vec = pindf_vector_to_std_vector<pindf_pdf_obj*>(names->content.array);
    
    for (size_t i = 0; i < vec.size(); i += 2) {
        pindf_pdf_obj *name = vec[i];
        pindf_pdf_obj *dest = vec[i + 1];

        name = deref(doc, name);
        dest = deref(doc, dest);

        if (name->obj_type != PINDF_PDF_LTR_STR && name->obj_type != PINDF_PDF_HEX_STR) {
            std::cerr << "Name is not a string, but " << name->obj_type << std::endl;
            continue;
        }
        pindf_uchar_str *name_uchar_str = name->content.ltr_str;
        std::string name_str = std::string((char*)name_uchar_str->p, name_uchar_str->len);
        
        names_to_page[name_str] = dest;
    }
}

void NameTree::print() {
    for (auto& [name, value] : names_to_page) {
        std::cout << name << " -> ";
        log_obj(value);
        std::cout << std::endl;
    }
}

NameTree *get_name_tree(pindf_doc *doc) {

    pindf_pdf_obj *catalog = get_catalog(doc);
    if (catalog == nullptr) {
        std::cerr << "Catalog is null" << std::endl;
        return nullptr;
    }

    NameTree *name_tree = new NameTree();

    // pdf 1.1 - /Dest
    pindf_pdf_obj *dests = pindf_dict_getvalue2(&catalog->content.dict, "/Dest");
    dests = deref(doc, dests);
    if (dests != nullptr && dests->obj_type == PINDF_PDF_DICT) {
        name_tree->from_obj(doc, dests);
    }
    
    // pdf 1.2 - /Names -> /Dests
    pindf_pdf_obj *names = pindf_dict_getvalue2(&catalog->content.dict, "/Names");
    names = deref(doc, names);
    if (names != nullptr && names->obj_type == PINDF_PDF_DICT) {
        pindf_pdf_obj *dests2 = pindf_dict_getvalue2(&names->content.dict, "/Dests");
        name_tree->from_obj(doc, dests2);
    }

    return name_tree;
}

pindf_pdf_obj* NameTree::get_dest(const std::string& name) {
    auto it = names_to_page.find(name);
    if (it == names_to_page.end()) {
        return nullptr;
    }
    return it->second;
}

nlohmann::json _to_json(pindf_doc *doc, pindf_pdf_obj *obj) {
    obj = deref(doc, obj);
    if (obj == nullptr) {
        std::cerr << "Name tree object is null" << std::endl;
        return {};
    }

    pindf_pdf_obj *kids = pindf_dict_getvalue2(&obj->content.dict, "/Kids");
    kids = deref(doc, kids);

    // not a leaf
    if (kids != nullptr) {
        nlohmann::json j = nlohmann::json::array();
        if (kids->obj_type != PINDF_PDF_ARRAY) {
            std::cerr << "Kids is not an array" << std::endl;
            return {};
        }
        std::vector<pindf_pdf_obj*> vec = pindf_vector_to_std_vector<pindf_pdf_obj*>(kids->content.array);
        for (pindf_pdf_obj *kid : vec) {
            j.push_back(_to_json(doc, kid));
        }
        return j;
    }

    // leaf
    pindf_pdf_obj *names = pindf_dict_getvalue2(&obj->content.dict, "/Names");
    names = deref(doc, names);
    if (names == nullptr) {
        std::cerr << "Names is null" << std::endl;
        return {};
    }
    
    nlohmann::json j;
    j["obj"] = obj_to_string(obj);
    return j;
}

nlohmann::json NameTree::to_json(pindf_doc *doc) {
    return _to_json(doc, this->name_tree_obj);
}