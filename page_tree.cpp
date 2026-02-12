#include "page_tree.hpp"
#include <iostream>

void PageMap::from_obj(pindf_doc* doc, pindf_pdf_obj* obj) {
    // basic check
    pindf_pdf_obj *orig_obj = obj;
    obj = deref(doc, obj);
    if (obj == nullptr) {
        std::cerr << "Failed to dereference object" << std::endl;
        return;
    }
    if (obj->obj_type != PINDF_PDF_DICT) {
        std::cerr << "Expected dictionary, got " << obj->obj_type << std::endl;
        return;
    }

    // type check
    pindf_pdf_obj *type = pindf_dict_getvalue2(&obj->content.dict, "/Type");
    if (type == nullptr) {
        std::cerr << "Expected /Type key in page tree node" << std::endl;
        return;
    }
    if (type->obj_type != PINDF_PDF_NAME) {
        std::cerr << "Expected name object for /Type key" << std::endl;
        return;
    }

    if (pindf_uchar_str_cmp3(type->content.name, "/Pages") == 0) {
        pindf_pdf_obj *kids = pindf_dict_getvalue2(&obj->content.dict, "/Kids");
        // kids is an array of references to page objects
        kids = deref(doc, kids);
        if (kids == nullptr) {
            std::cerr << "Expected /Kids key in page tree node" << std::endl;
            return;
        }
        if (kids->obj_type != PINDF_PDF_ARRAY) {
            std::cerr << "Expected array object for /Kids key" << std::endl;
            return;
        }

        std::vector<pindf_pdf_obj*> kids_vec = pindf_vector_to_std_vector<pindf_pdf_obj*>(kids->content.array);
        for (auto kid : kids_vec) {
            from_obj(doc, kid);
        }
    } else if (pindf_uchar_str_cmp3(type->content.name, "/Page") == 0) {
        int temp_obj_num = orig_obj->content.ref.obj_num;
        obj_nums.push_back(temp_obj_num);
        obj_num_to_page_index[temp_obj_num] = obj_nums.size();
    } else {
        std::cerr << "Expected /Type to be /Pages" << std::endl;
    }
}

int PageMap::get_index(int obj_num) {
    if (obj_num_to_page_index.find(obj_num) != obj_num_to_page_index.end()) {
        return obj_num_to_page_index[obj_num];
    }
    return -1;
}

PageMap *get_pages(pindf_doc *doc) {
    pindf_pdf_obj *catalog = get_catalog(doc);
    if (catalog == nullptr) {
        std::cerr << "Failed to get catalog" << std::endl;
        return nullptr;
    }

    pindf_pdf_obj *pages = pindf_dict_getvalue2(&catalog->content.dict, "/Pages");
    if (pages == nullptr) {
        std::cerr << "Failed to get pages" << std::endl;
        return nullptr;
    }
    
    PageMap *page_map = new PageMap();
    page_map->from_obj(doc, pages);
    return page_map;
}