#!/bin/bash

sed -i 's|fmt::format|std::format|g' *.[ch]pp *.cpp.template
sed -i 's|fmt/format\.h|format|g' *.[ch]pp *.cpp.template
sed -i 's|u8string|string|g' *.[ch]pp *.cpp.template
sed -i 's|u8"|"|g' *.[ch]pp *.cpp.template
sed -i 's|char8_t|char|g' *.[ch]pp *.cpp.template
