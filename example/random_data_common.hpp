#pragma once

#include <fstream>
#include <vector>

template <typename Elem>
void Read(std::istream &in, std::vector<Elem> &dest, std::size_t count) {
  dest.resize(count);
  in.read(reinterpret_cast<char *>(dest.data()),
          static_cast<std::streamsize>(count * sizeof(Elem)));
}

template <typename Elem>
void Write(std::ostream &out, std::vector<Elem> &src) {
  out.write(reinterpret_cast<char *>(src.data()),
            static_cast<std::streamsize>(src.size() * sizeof(Elem)));
}
