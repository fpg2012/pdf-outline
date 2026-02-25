#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "page_tree.hpp"
#include "name_tree.hpp"
#include "outline.hpp"

enum Mode {
    Extract,
    Replace
};

enum ErrorNO {
    OK,
    FileOpenFailed,
    ParseFailed,
    SaveFailed,
    UnknownError,
};

struct Opts {
    std::string pdf_file;
    std::string output_file;
    std::string json_file;
    Mode mode;
};

void help() {
    std::cout << "Usage:\n";
    std::cout << "  pdf-outline <pdf_file> -x <output.json> [-o <output.pdf>]   Extract outline from PDF\n";
    std::cout << "  pdf-outline <pdf_file> -r <input.json> -o <output.pdf>      Replace outline in PDF\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help               Show this help message\n";
    std::cout << "  -x <output.json>         Extract outline to JSON file\n";
    std::cout << "  -r <input.json>          Replace outline from JSON file\n";
    std::cout << "  -o <output.pdf>          Output PDF file (required for replace mode, optional for extract)\n";
}

int extract(const std::string &pdf_file, const std::string &output_file) {
    pindf_doc *doc = parse_pdf(pdf_file.c_str());
    if (!doc) {
        std::cerr << "Error: Could not parse PDF file\n";
        return ParseFailed;
    }
    
    PageMap *page_map = get_pages(doc);
    NameTree *name_tree = get_name_tree(doc);
    OutlineNode *outline = get_outline(doc, name_tree, page_map);
    
    std::ofstream out(output_file, std::ios::out | std::ios::binary);
    out << outline->to_json().dump(2);

    out.close();
    delete page_map;
    delete name_tree;
    delete outline;
    pindf_doc_destroy(doc);
    free(doc);

    return OK;
}

int replace(const std::string &pdf_file, const std::string &input_file, const std::string &output_file) {
    // open file
    FILE *in_pdf = fopen(pdf_file.c_str(), "rb");
    if (!in_pdf) {
        std::cerr << "Error: Could not open input PDF file\n";
        return FileOpenFailed;
    }
    FILE *out_pdf = fopen(output_file.c_str(), "wb");
    if (!out_pdf) {
        std::cerr << "Error: Could not open output PDF file\n";
        fclose(in_pdf);
        return FileOpenFailed;
    }

    std::ifstream in_json(input_file, std::ios::in);
    
    // copy file
    char *buffer = new char[4096];
    size_t n;
    while ((n = fread(buffer, 1, 4096, in_pdf)) > 0) {
        fwrite(buffer, 1, n, out_pdf);
    }
    delete[] buffer;
    fclose(in_pdf);

    // parse doc
    pindf_doc *doc = parse_pdf(pdf_file.c_str());

    PageMap *page_map = get_pages(doc);

    OutlineNode new_outline;
    nlohmann::json j;
    in_json >> j;
    new_outline.from_json(j, page_map);

    pindf_modif *modif = pindf_modif_new(doc->xref->size + 1);
    new_outline.apply_modif(doc, modif, page_map);
    doc->modif = modif;

    int ret = pindf_doc_save_modif(doc, out_pdf, true);
    if (ret != 0) {
        std::cerr << "Error: Could not save modified PDF file\n";
        fclose(out_pdf);
        return SaveFailed;
    }

    fclose(out_pdf);
    pindf_doc_destroy(doc);
    free(doc);
    delete page_map;
    in_json.close();

    return OK;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        help();
        return 1;
    }

    Opts opts;
    bool has_mode = false;
    bool has_output = false;

    // First argument is PDF file (positional)
    opts.pdf_file = argv[1];

    // Parse options
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help();
            return 0;
        } else if (strcmp(argv[i], "-x") == 0) {
            if (has_mode) {
                std::cerr << "Error: Cannot specify both -x and -r\n";
                return 1;
            }
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -x\n";
                return 1;
            }
            opts.mode = Extract;
            opts.json_file = argv[++i]; // output.json for extract
            has_mode = true;
        } else if (strcmp(argv[i], "-r") == 0) {
            if (has_mode) {
                std::cerr << "Error: Cannot specify both -x and -r\n";
                return 1;
            }
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -r\n";
                return 1;
            }
            opts.mode = Replace;
            opts.json_file = argv[++i]; // input.json for replace
            has_mode = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing argument for -o\n";
                return 1;
            }
            opts.output_file = argv[++i];
            has_output = true;
        } else {
            std::cerr << "Error: Unknown option '" << argv[i] << "'\n";
            help();
            return 1;
        }
    }

    // Validation
    if (!has_mode) {
        std::cerr << "Error: Must specify either -x or -r\n";
        help();
        return 1;
    }

    if (opts.mode == Replace && !has_output) {
        std::cerr << "Error: Replace mode requires -o option\n";
        help();
        return 1;
    }

    // For extract mode, if no output PDF specified, use input PDF name with _out suffix
    if (opts.mode == Extract && !has_output) {
        size_t dot = opts.pdf_file.find_last_of('.');
        if (dot != std::string::npos) {
            opts.output_file = opts.pdf_file.substr(0, dot) + "_out.pdf";
        } else {
            opts.output_file = opts.pdf_file + "_out.pdf";
        }
    }

    // Debug print
    std::cout << "PDF file: " << opts.pdf_file << "\n";
    std::cout << "Mode: " << (opts.mode == Extract ? "Extract" : "Replace") << "\n";
    std::cout << "Input file (JSON): " << opts.json_file << "\n";
    std::cout << "Output PDF: " << opts.output_file << "\n";

    // TODO: Implement actual PDF outline extraction/replacement
    int ret;
    if (opts.mode == Replace) {
        ret = replace(opts.pdf_file, opts.json_file, opts.output_file);
    } else if (opts.mode == Extract) {
        ret = extract(opts.pdf_file, opts.json_file);
    } else {
        ret = UnknownError;
    }

    return ret;
}