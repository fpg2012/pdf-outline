#include "destination.hpp"
#include "page_tree.hpp"
#include "utils.hpp"
#include <iostream>

void Destination::from_obj(pindf_doc *doc, NameTree *name_tree, PageMap *page_map, pindf_pdf_obj *dest) {
    assert(doc != nullptr);

    dest = deref(doc, dest);
    dest_obj = dest;

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
        // dest_obj = dest;
        return;
    } else {
        std::cerr << "Unknown destination type: " << dest->obj_type << std::endl;
        return;
    }
}

nlohmann::json Destination::to_json() const {
    nlohmann::json j;
    j["dest_arr"] = obj_to_string(dest_obj);
    j["page"] = page;
    if (!name.empty()) {
        j["name"] = name;
    }
    return j;
}

void Destination::from_json(const nlohmann::json &j, const PageMap *page_map) {
    if (j.contains("page")) {
        page = j["page"];
    }
    if (j.contains("name")) {
        name = j["name"];
    }
    // std::string page_ref_str = j["page_ref"];
    std::string dest_arr_str = j["dest_arr"];
    
    auto parser = pindf_parser_new();
    auto lexer = pindf_lexer_new();

    std::cout << "parsing: " << dest_arr_str << std::endl;

    pindf_pdf_obj *obj = nullptr;
    // auto buffer = pindf_uchar_str_from_cstr(page_ref_str.c_str(), page_ref_str.length());
    // pindf_parse_one_obj_from_buffer(parser, lexer, buffer, 0, &obj, NULL, PINDF_PDF_REF);
    // pindf_uchar_str_destroy(buffer);
    // free(buffer);
    // page_obj = obj;

    auto buffer = pindf_uchar_str_from_cstr(dest_arr_str.c_str(), dest_arr_str.length() + 1);
    pindf_parse_one_obj_from_buffer(parser, lexer, buffer, 0, &obj, NULL, PINDF_PDF_ARRAY);
    pindf_uchar_str_destroy(buffer);
    free(buffer);
    dest_obj = obj;
    dest_arr = pindf_vector_to_std_vector<pindf_pdf_obj*>(dest_obj->content.array);

    // clean up
    pindf_lexer_clear(lexer);
    pindf_parser_destroy(parser);
    free(parser);
    free(lexer);
}

pindf_pdf_obj *Destination::to_obj(pindf_doc* doc, const PageMap *page_map) const {
    pindf_pdf_obj *obj = pindf_pdf_obj_new(PINDF_PDF_ARRAY);
    
    int page_obj_num = page_map->obj_nums[page-1];

    if (page_obj_num == dest_arr[0]->content.ref.obj_num) {
        // no modification
        return dest_obj;
    }

    auto page_ref = pindf_pdf_obj_new(PINDF_PDF_REF);
    page_ref->content.ref = (pindf_pdf_ref){
        .obj_num = page_obj_num,
        .generation_num = 0,
    };

    auto *vec = pindf_vector_new(4, sizeof(pindf_pdf_obj*));
    pindf_vector_append(vec, &page_ref);
    for (int i = 1; i < 4; ++i) {
        pindf_vector_append(vec, (void*)(dest_arr.data() + i));
    }

    obj->content.array = vec;
    return obj;
}