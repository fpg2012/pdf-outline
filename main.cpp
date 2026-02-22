#include <cassert>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>

#include "utils.hpp"
#include "name_tree.hpp"
#include "outline.hpp"
#include "page_tree.hpp"
#include <nlohmann/json.hpp>

extern "C" {
#include "pindf/pindf.h"
#include "pindf/logger/logger.h"
}

int main(int argc, const char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [pdf_fileanme]" << std::endl;
        return -1;
    }

#ifdef NDEBUG
    pindf_set_log_level(PINDF_LOG_LEVEL_INFO);
#else
    pindf_set_log_level(PINDF_LOG_LEVEL_INFO);
#endif

    pindf_doc *doc = parse_pdf(argv[1]);
    if (doc == nullptr) {
        std::cerr << "Failed to parse PDF file" << std::endl;
        return -1;
    }

    std::cout << "Parsed PDF successfully" << std::endl;
    std::cout << "xref_offset: " << doc->xref_offset << std::endl;
    std::cout << "trailer entries " << doc->trailer.keys->len << std::endl;

    std::cout << "=== " << "print outline 1" << " ===" << std::endl;
    print_outline(doc);


    std::cout << "=== " << "page map" << " ===" << std::endl;
    PageMap *page_map = get_pages(doc);
    for (auto &[obj_num, index] : page_map->obj_num_to_page_index) {
        std::cout << obj_num << "\t-> " << index << std::endl;
    }

    // name tree
    std::cout << "=== " << "name tree" << " ===" << std::endl;
    NameTree *name_tree = get_name_tree(doc);
    name_tree->print();

    std::cout << "=== " << "print outline 2" << " ===" << std::endl;
    OutlineNode *outline = get_outline(doc, name_tree, page_map);
    outline->print();

    // save to json
    std::ofstream out("outline.json");
    out << outline->to_simple_json().dump(2);
    out.close();
    out.open("outline_full.json");
    out << outline->to_json().dump(2);
    out.close();
    
    // try load from json
    std::cout << "=== " << "parse outline from json" << " ===" << std::endl;
    OutlineNode new_outline;
    std::fstream json_file("outline_full.json");
    nlohmann::json j;
    json_file >> j;
    new_outline.from_json(j);
    new_outline.print();

    // save name_tree to json
    std::cout << "=== " << "save name tree to json" << " ===" << std::endl;
    std::ofstream nt_file("nt.json");
    auto nt_j = name_tree->to_json(doc);
    nt_file << nt_j.dump(2);

    // clean up
    delete name_tree;
    delete outline;
    delete page_map;
    return 0;
}
