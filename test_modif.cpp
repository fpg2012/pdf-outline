#include <cassert>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>

#include "utils.hpp"
#include "outline.hpp"
#include "page_tree.hpp"
#include <nlohmann/json.hpp>

extern "C" {
#include "pindf/pindf.h"
#include "pindf/logger/logger.h"
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " [pdf_fileanme] [outline_json]" << std::endl;
        return -1;
    }

    pindf_doc *doc = parse_pdf(argv[1]);
    if (doc == nullptr) {
        std::cerr << "Failed to parse PDF file" << std::endl;
        return -1;
    }

    PageMap *page_map = get_pages(doc);

    OutlineNode new_outline;
    std::fstream json_file(argv[2]);
    nlohmann::json j;
    json_file >> j;
    new_outline.from_json(j, page_map);
    
    pindf_modif *modif = pindf_modif_new(doc->xref->size + 1);
    new_outline.apply_modif(doc, modif, page_map);
    doc->modif = modif;

    std::ofstream json_output("outline_modif.json");
    json_output << new_outline.to_json().dump(2);
    json_output.close();

    // print modif xref table
    FILE *modif_fp = fopen("modif_result.pdf", "wb");
    FILE *orig_fp = fopen(argv[1], "rb");
    char buffer[1024];
    // copy
    while (1) {
        size_t n = fread(buffer, 1, sizeof(buffer), orig_fp);
        if (n == 0) {
            break;
        }
        fwrite(buffer, 1, n, modif_fp);
    }
    fclose(orig_fp);
    
    pindf_doc_save_modif(doc, modif_fp, true);
    fclose(modif_fp);
}