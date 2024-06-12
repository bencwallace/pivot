#include <dlfcn.h>
#include <iostream>

#include "graphviz.h"

namespace pivot {

gvc_t &gvc_t::load() {
  static gvc_t instance;
  return instance;
}

gvc_t::gvc_t() {
  void *handle = dlopen("libgvc.so", RTLD_LAZY);
  if (!handle) {
    std::cerr << "Error loading graphviz. Make sure graphviz is installed and in your LD_LIBRARY_PATH." << std::endl;
    throw std::runtime_error(dlerror());
  }
  gvContext = reinterpret_cast<gvContext_t>(dlsym(handle, "gvContext"));
  gvLayout = reinterpret_cast<gvLayout_t>(dlsym(handle, "gvLayout"));
  gvRenderFilename = reinterpret_cast<gvRenderFilename_t>(dlsym(handle, "gvRenderFilename"));
  gvFreeLayout = reinterpret_cast<gvFreeLayout_t>(dlsym(handle, "gvFreeLayout"));
  gvFreeContext = reinterpret_cast<gvFreeContext_t>(dlsym(handle, "gvFreeContext"));
}

cgraph_t &cgraph_t::load() {
  static cgraph_t instance;
  return instance;
}

cgraph_t::cgraph_t() {
  void *handle = dlopen("libcgraph.so", RTLD_LAZY);
  if (!handle) {
    std::cerr << "Error loading graphviz. Make sure graphviz is installed and in your LD_LIBRARY_PATH." << std::endl;
    throw std::runtime_error(dlerror());
  }
  agopen = reinterpret_cast<agopen_t *>(dlsym(handle, "agopen"));
  agattr = reinterpret_cast<agattr_t *>(dlsym(handle, "agattr"));
  agclose = reinterpret_cast<agclose_t *>(dlsym(handle, "agclose"));
  agnode = reinterpret_cast<agnode_t *>(dlsym(handle, "agnode"));
  agset = reinterpret_cast<agset_t *>(dlsym(handle, "agset"));
  agedge = reinterpret_cast<agedge_t *>(dlsym(handle, "agedge"));
  this->Agdirected = *reinterpret_cast<Agdesc_t *>(dlsym(handle, "Agdirected"));
}

} // namespace pivot
