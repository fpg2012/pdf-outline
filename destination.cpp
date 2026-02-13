#include "destination.hpp"
#include "utils.hpp"
#include <iostream>

void Destination::from_obj(pindf_doc *doc, NameTree *name_tree, PageMap *page_map, pindf_pdf_obj *dest) {
    assert(doc != nullptr);

    dest = deref(doc, dest);

    if (dest == nullptr || (dest->obj_type != PINDF_PDF_ARRAY && dest->obj_type != PINDF_PDF_LTR_STR && dest->obj_type != PINDF_PDF_HEX_STR)) {
        return;
    }

    if (dest->obj_type == PINDF_PDF_ARRAY) {
        dest_arr = pindf_vector_to_std_vector<pindf_pdf_obj*>(dest->content.array);
        page_obj = dest_arr[0];
        
        if (page_obj == nullptr) {
            std::cerr << "Failed to extract page number, dest[0] is null" << std::endl;
            return;
        } else if (page_obj->obj_type != PINDF_PDF_REF) {
            std::cerr << "Failed to extract page number, dest[0] is not an ref, but " << page_obj->obj_type << std::endl;
            log_obj(page_obj);
            return;
        }

        int obj_num = page_obj->content.ref.obj_num;
        page = page_map->get_index(obj_num);
        return;
    } else if (dest->obj_type == PINDF_PDF_LTR_STR || dest->obj_type == PINDF_PDF_HEX_STR) {
        // handle named destination
        name = pindf_uchar_str_to_string(dest->content.ltr_str);
        pindf_pdf_obj *final_dest = name_tree->get_dest(name);

        if (final_dest == nullptr || final_dest->obj_type != PINDF_PDF_DICT) {
            std::cerr << "cannot find key: " << name << std::endl;
            return;
        }
        dest = pindf_dict_getvalue2(&final_dest->content.dict, "/D");
        from_obj(doc, name_tree, page_map, dest);
        return;
    } else {
        std::cerr << "Unknown destination type: " << dest->obj_type << std::endl;
        return;
    }
}