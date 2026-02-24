#include "outline.hpp"
#include "nlohmann/json.hpp"
#include "utils.hpp"
#include <stdexcept>

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
    }

    // a leaf
    pindf_pdf_obj *title_obj = pindf_dict_getvalue2(&obj->content.dict, "/Title");
    title_obj = deref(doc, title_obj);
    if (title_obj != nullptr) {
        if (title_obj->obj_type == PINDF_PDF_LTR_STR || title_obj->obj_type == PINDF_PDF_HEX_STR) {
            title = decode_text_string(title_obj->content.ltr_str);
        } else {
            std::cerr << "Warning: outline node title is not a string, but " << title_obj->obj_type << std::endl;
        }
    } else {
        std::cerr << "Warning: outline node has no title" << std::endl;
    }

    // direct destination
    pindf_pdf_obj *dest = pindf_dict_getvalue2(&obj->content.dict, "/Dest");
    if (dest != nullptr) {
        destination.from_obj(doc, name_tree, page_map, dest);
        return;
    }

    // action
    pindf_pdf_obj *action = pindf_dict_getvalue2(&obj->content.dict, "/A");
    action = deref(doc, action);
    if (action == nullptr) {
        std::cerr << "Warning: outline node has no action" << std::endl;
        return;
    }
    pindf_pdf_obj *s = pindf_dict_getvalue2(&action->content.dict, "/S");
    s = deref(doc, s);

    if (s == nullptr) {
        std::cerr << "Warning: outline node action has no S" << std::endl;
        return;
    }

    if (s->obj_type != PINDF_PDF_NAME || pindf_uchar_str_cmp3(s->content.name, "/GoTo") != 0) {
        std::cerr << "Warning: outline node action S is not a name" << std::endl;
        return;
    }
    
    pindf_pdf_obj *d = pindf_dict_getvalue2(&action->content.dict, "/D");
    d = deref(doc, d);
    destination.from_obj(doc, name_tree, page_map, d);
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
    std::cout << std::string(depth * 4, ' ') << "Page: " << destination.page << std::endl;
    for (auto& child : children) {
        child.print(depth + 1);
    }
}

pindf_pdf_obj *get_outline_obj(pindf_doc *doc) {
    // get root/catalog
    pindf_pdf_obj *root = pindf_dict_getvalue2(&doc->trailer, "/Root");
    root = deref(doc, root);
    if (root == nullptr) {
        std::cerr << "Failed to get root object" << std::endl;
        return nullptr;
    }

    // get outlines
    pindf_pdf_obj *outlines = pindf_dict_getvalue2(&root->content.dict, "/Outlines");
    if (outlines == nullptr) {
        std::cerr << "Failed to get outlines object" << std::endl;
        return nullptr;
    }
    return outlines;
}

OutlineNode *get_outline(pindf_doc *doc, NameTree *name_tree, PageMap *page_map) {
    pindf_pdf_obj *outlines = get_outline_obj(doc);
    if (outlines == nullptr) {
        return nullptr;
    }

    outlines = deref(doc, outlines);
    
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

nlohmann::json OutlineNode::to_simple_json() const {
    nlohmann::json j;
    if (!children.empty()) {
        j = nlohmann::json::array();
        for (const auto &child : children) {
            j.push_back(child.to_simple_json());
        }
        return j;
    }
    if (!title.empty()) {
        j["title"] = title;
    }
    if (destination.page != -1) {
        j["page"] = destination.page;
    }
    return j;
}

nlohmann::json OutlineNode::to_json() const {
    nlohmann::json j;
    if (!title.empty()) {
        j["title"] = title;
    }
    if (!children.empty()) {
        nlohmann::json chd = nlohmann::json::array();
        for (const auto &child : children) {
            chd.push_back(child.to_json());
        }
        j["chd"] = chd;
    }
    if (destination.page != -1) {
        j["destination"] = destination.to_json();
    }
    return j;
}

void OutlineNode::from_json(const nlohmann::json &j, const PageMap *page_map) {
    if (j.is_object()) {
        if (j.contains("title")) {
            title = j["title"];
        }
        if (j.contains("destination")) {
            destination.from_json(j["destination"], page_map);
        }
        if (j.contains("chd")) {
            for (const auto &child : j["chd"]) {
                OutlineNode node;
                node.from_json(child, page_map);
                children.push_back(node);
            }
        }
    }
}

int to_temp(const OutlineNode *outline_node, std::vector<TempOutlineNode*> &nodes, int offset) {
    TempOutlineNode *node = new TempOutlineNode();
    std::vector<TempOutlineNode*> temp_vec;
    if (outline_node->children.size() > 0) {
        auto new_offset = offset;
        for (int i = 0; i < outline_node->children.size(); ++i) {
            new_offset = to_temp(&outline_node->children[i], nodes, new_offset);
            temp_vec.push_back(nodes[new_offset]);
        }
        for (int i = 0; i < temp_vec.size(); ++i) {
            if (i > 0)
                temp_vec[i]->last = temp_vec[i-1]->offset;
            if (i < temp_vec.size() - 1)
                temp_vec[i]->next = temp_vec[i+1]->offset;
        }
        node->first = temp_vec[0]->offset;
        node->last = temp_vec[temp_vec.size() - 1]->offset;
    }
    node->node = outline_node;
    node->offset = nodes.size();
    node->parent = node->offset;
    for (auto child : temp_vec) {
        child->parent = node->offset;
    }
    nodes.push_back(node);
    return node->offset;
}

void OutlineNode::apply_modif(pindf_doc *doc, pindf_modif *modif, const PageMap *page_map) const {
    assert(doc != nullptr);
    assert(modif != nullptr);

    std::vector<TempOutlineNode*> nodes;
    to_temp(this, nodes, modif->max_obj_num);

    int real_offset = modif->max_obj_num + 1;

    int obj_num_last = 0;
    
    for (auto node : nodes) {
        pindf_pdf_obj *obj = pindf_pdf_obj_new(PINDF_PDF_DICT);
        pindf_pdf_dict temp_dict;
        pindf_pdf_dict_init(&temp_dict);

        {
            if (!node->node->title.empty()) {
                auto encoded_title = encode_text_string(node->node->title);
                pindf_dict_set_value2(&temp_dict, "/Title", to_obj<const std::string&>(encoded_title));
            }

            if (node->first != -1) {
                pindf_pdf_obj *first_obj = pindf_pdf_obj_new(PINDF_PDF_REF);
                first_obj->content.ref = {
                    .obj_num = real_offset + node->first,
                    .generation_num = 0
                };
                pindf_dict_set_value2(&temp_dict, "/First", first_obj);

                pindf_pdf_obj *last_obj = pindf_pdf_obj_new(PINDF_PDF_REF);
                last_obj->content.ref = {
                    .obj_num = real_offset + node->last,
                    .generation_num = 0,
                };
                pindf_dict_set_value2(&temp_dict, "/Last", last_obj);

                pindf_pdf_obj *count_obj = to_obj<int>(children.size());
                pindf_dict_set_value2(&temp_dict, "/Count", count_obj);
            }

            pindf_pdf_obj *parent_obj = pindf_pdf_obj_new(PINDF_PDF_REF);
            parent_obj->content.ref = {
                .obj_num = real_offset + node->parent,
                .generation_num = 0,
            };
            pindf_dict_set_value2(&temp_dict, "/Parent", parent_obj);

            // set next
            if (node->next != -1) {
                pindf_pdf_obj *next_obj = pindf_pdf_obj_new(PINDF_PDF_REF);
                next_obj->content.ref = {
                    .obj_num = real_offset + node->next,
                    .generation_num = 0,
                };
                pindf_dict_set_value2(&temp_dict, "/Next", next_obj);
            }
            // set prev
            if (node->prev != -1) {
                pindf_pdf_obj *prev_obj = pindf_pdf_obj_new(PINDF_PDF_REF);
                prev_obj->content.ref = {
                    .obj_num = real_offset + node->prev,
                    .generation_num = 0,
                };
                pindf_dict_set_value2(&temp_dict, "/Prev", prev_obj);
            }
            // set dest
            if (node->node->destination.page != -1) {
                pindf_pdf_obj *dest_obj = node->node->destination.to_obj(doc, page_map);
                pindf_dict_set_value2(&temp_dict, "/Dest", dest_obj);
            }

            // set type
            if (node->offset == node->parent) {
                pindf_pdf_obj *type_obj = pindf_pdf_obj_new(PINDF_PDF_NAME);
                pindf_uchar_str *str = pindf_uchar_str_from_cstr("/Outlines", strlen("/Outlines"));
                type_obj->content.name = str;
                pindf_dict_set_value2(&temp_dict, "/Type", type_obj);
            }
        }

        obj->content.dict = temp_dict;
        
        pindf_pdf_ind_obj *ind_obj = new pindf_pdf_ind_obj{
            .obj_num = real_offset + node->offset,
            .generation_num = 0,
            .obj = obj,
            .start_pos = 0,
        };
        obj_num_last = real_offset + node->offset;
        
        if (node->offset == node->parent) {
            // get original outline object num
            auto *orig_outline_obj = get_outline_obj(doc);
            int orig_outline_obj_num = orig_outline_obj->content.ref.obj_num;
            ind_obj->obj_num = orig_outline_obj_num;
            pindf_modif_addentry(modif, ind_obj, orig_outline_obj_num);
        } else {
            pindf_modif_addentry(modif, ind_obj, real_offset + node->offset);
        }
    }
}