#include <cassert>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include "utils.hpp"
#include "name_tree.hpp"
#include "outline.hpp"
#include "page_tree.hpp"

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

    // clean up
    delete name_tree;
    delete outline;
    delete page_map;
    return 0;
}
