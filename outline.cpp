#include "outline.hpp"

void OutlineNode::from_obj(pindf_doc *doc, NameTree *name_tree, PageMap *page_map, pindf_pdf_obj *obj) {
    assert(name_tree != nullptr);

    if (obj == nullptr || obj->obj_type != PINDF_PDF_DICT) {
        return;
    }

    // parser childrens
    pindf_pdf_obj *first = pindf_dict_getvalue2(&obj->content.dict, "/First");
    if (first != nullptr) {
        first = deref(doc, first);
        while (first != nullptr) {
            OutlineNode node;
            node.from_obj(doc, name_tree, page_map, first);
            children.push_back(node);
            pindf_pdf_obj *next = pindf_dict_getvalue2(&first->content.dict, "/Next");
            next = deref(doc, next);
            first = next;
        }
        return;
    }

    // a leaf
    pindf_pdf_obj *title_obj = pindf_dict_getvalue2(&obj->content.dict, "/Title");
    title_obj = deref(doc, title_obj);
    if (title_obj != nullptr) {
        if (title_obj->obj_type == PINDF_PDF_LTR_STR || title_obj->obj_type == PINDF_PDF_HEX_STR) {
            // title = pindf_uchar_str_to_string(title_obj->content.ltr_str);
            title = decode_text_string(title_obj->content.ltr_str);
        } else {
            std::cerr << "Warning: outline node title is not a string, but " << title_obj->obj_type << std::endl;
        }
    } else {
        std::cerr << "Warning: outline node has no title" << std::endl;
    }

    // parse page
    page = -1;
    pindf_pdf_obj *dest = pindf_dict_getvalue2(&obj->content.dict, "/Dest");
    if (dest != nullptr && dest->obj_type == PINDF_PDF_ARRAY) {
        page = extract_page_number(doc, page_map, dest);
    } else {
        // action
        pindf_pdf_obj *action = pindf_dict_getvalue2(&obj->content.dict, "/A");
        action = deref(doc, action);
        if (action != nullptr) {
            pindf_pdf_obj *s = pindf_dict_getvalue2(&action->content.dict, "/S");
            s = deref(doc, s);
            
            if (s != nullptr) {
                if (s->obj_type == PINDF_PDF_NAME && pindf_uchar_str_cmp3(s->content.name, "/GoTo") == 0) {
                    pindf_pdf_obj *d = pindf_dict_getvalue2(&action->content.dict, "/D");
                    d = deref(doc, d);
                    if (d != nullptr && d->obj_type == PINDF_PDF_ARRAY) {
                        page = extract_page_number(doc, page_map, d);
                    } else if (d != nullptr && (d->obj_type == PINDF_PDF_LTR_STR || d->obj_type == PINDF_PDF_HEX_STR)) {
                        std::string name = pindf_uchar_str_to_string(d->content.ltr_str);
                        pindf_pdf_obj *final_dest = name_tree->get_dest(name);
                        if (final_dest != nullptr && final_dest->obj_type == PINDF_PDF_DICT) {
                            pindf_pdf_obj *d = pindf_dict_getvalue2(&final_dest->content.dict, "/D");
                            d = deref(doc, d);
                            if (d != nullptr && d->obj_type == PINDF_PDF_ARRAY) {
                                page = extract_page_number(doc, page_map, d);
                            } else {
                                std::cerr << "destination is not an array" << std::endl;
                            }
                        } else {
                            std::cerr << "cannot find key: " << name << std::endl;
                        }
                    }
                } else {
                    std::cerr << "Warning: outline node action is not GoTo" << std::endl;
                }
            }
        }
    }
}

void _print_outline(pindf_doc *doc, pindf_pdf_obj *outline_obj, int depth = 0)
{
    std::cout << std::string(depth * 4, ' ');
    log_obj(outline_obj);

    pindf_pdf_obj *title = pindf_dict_getvalue2(&outline_obj->content.dict, "/Title");
    title = deref(doc, title);
    if (title != nullptr) {
        std::cout << "title: ";
        log_obj(title);
    }

    pindf_pdf_obj *dest = pindf_dict_getvalue2(&outline_obj->content.dict, "/Dest");
    dest = deref(doc, dest);
    if (dest == nullptr) {
        pindf_pdf_obj *action = pindf_dict_getvalue2(&outline_obj->content.dict, "/A");
        action = deref(doc, action);
        if (action != nullptr) {
            std::cout << std::string(depth * 4, ' ') << "\033[33mAction: ";
            log_obj(action);
            std::cout << "\033[0m" << std::endl;
        }
    }

    pindf_pdf_obj *first = pindf_dict_getvalue2(&outline_obj->content.dict, "/First");
    first = deref(doc, first);

    while (first != nullptr) {
        _print_outline(doc, first, depth + 1);
        first = pindf_dict_getvalue2(&first->content.dict, "/Next");
        first = deref(doc, first);
    }
}

void print_outline(pindf_doc *doc) {
    assert(doc != nullptr);

    // get root/catalog
    pindf_pdf_obj *root = pindf_dict_getvalue2(&doc->trailer, "/Root");
    root = deref(doc, root);
    if (root == nullptr) {
        std::cerr << "Failed to get root object" << std::endl;
        return;
    }

    // print catalog
    std::cout << "Catalog: " << std::endl;
    log_obj(root);
    std::cout << std::endl;

    // get outlines
    pindf_pdf_obj *outlines = pindf_dict_getvalue2(&root->content.dict, "/Outlines");
    outlines = deref(doc, outlines);
    if (outlines == nullptr) {
        std::cerr << "Failed to get outlines object" << std::endl;
        return;
    }
    
    _print_outline(doc, outlines);
}

void OutlineNode::print(int depth) {
    std::cout << std::string(depth * 4, ' ') << "Title: " << title << std::endl;
    std::cout << std::string(depth * 4, ' ') << "Page: " << page << std::endl;
    for (auto& child : children) {
        child.print(depth + 1);
    }
}

OutlineNode *get_outline(pindf_doc *doc, NameTree *name_tree, PageMap *page_map) {
    // get root/catalog
    pindf_pdf_obj *root = pindf_dict_getvalue2(&doc->trailer, "/Root");
    root = deref(doc, root);
    if (root == nullptr) {
        std::cerr << "Failed to get root object" << std::endl;
        return nullptr;
    }

    // get outlines
    pindf_pdf_obj *outlines = pindf_dict_getvalue2(&root->content.dict, "/Outlines");
    outlines = deref(doc, outlines);
    if (outlines == nullptr) {
        std::cerr << "Failed to get outlines object" << std::endl;
        return nullptr;
    }
    
    OutlineNode *outline = new OutlineNode();
    outline->from_obj(doc, name_tree, page_map, outlines);
    return outline;
}

int extract_page_number(pindf_doc *doc, PageMap *page_map, pindf_pdf_obj *obj) {
    obj = deref(doc, obj);

    if (obj == nullptr || obj->obj_type != PINDF_PDF_ARRAY) {
        return -1;
    }

    // pindf_pdf_obj *d = pindf_dict_getvalue2(&obj->content.dict, "/D");
    // if (d == nullptr || d->obj_type != PINDF_PDF_ARRAY) {
    //     std::cerr << "Failed get /D in Dest object" << std::endl;
    //     return -1;
    // }

    std::vector<pindf_pdf_obj*> dest = pindf_vector_to_std_vector<pindf_pdf_obj*>(obj->content.array);
    int page = -1;
    
    pindf_pdf_obj *page_obj = dest[0];
    if (page_obj == nullptr) {
        std::cerr << "Failed to extract page number, dest[0] is null" << std::endl;
        return -1;
    } else if (page_obj->obj_type != PINDF_PDF_REF) {
        std::cerr << "Failed to extract page number, dest[0] is not an ref, but " << page_obj->obj_type << std::endl;
        log_obj(page_obj);
        return -1;
    }

    int obj_num = page_obj->content.ref.obj_num;
    page = page_map->get_index(obj_num);

    return page;
}