/*
 Copyright 2026 fpg2012 (aka nth233)

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#pragma once
#include <vector>
#include <unordered_map>

extern "C" {
#include "pindf/pindf.h"
}
#include "utils.hpp"

struct PageMap {
    std::vector<int> obj_nums;
    std::unordered_map<int, int> obj_num_to_page_index;

    void from_obj(pindf_doc* doc, pindf_pdf_obj* obj);
    int get_index(int obj_num);
};

PageMap *get_pages(pindf_doc *doc);
