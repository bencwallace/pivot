#pragma once

#include <gvc.h>

namespace pivot {

class gvc_t {

public:
  using gvContext_t = GVC_t *(*)();
  using gvLayout_t = int (*)(GVC_t *, graph_t *, const char *);
  using gvRenderFilename_t = int (*)(GVC_t *, graph_t *, const char *, const char *);
  using gvFreeLayout_t = int (*)(GVC_t *, graph_t *);
  using gvFreeContext_t = int (*)(GVC_t *);

  static gvc_t &load();

  gvContext_t gvContext;
  gvLayout_t gvLayout;
  gvRenderFilename_t gvRenderFilename;
  gvFreeLayout_t gvFreeLayout;
  gvFreeContext_t gvFreeContext;

private:
  gvc_t();
};

class cgraph_t {

public:
  using agopen_t = decltype(::agopen);
  using agattr_t = decltype(::agattr);
  using agclose_t = decltype(::agclose);
  using agnode_t = decltype(::agnode);
  using agset_t = decltype(::agset);
  using agedge_t = decltype(::agedge);

  static cgraph_t &load();

  agopen_t *agopen;
  agattr_t *agattr;
  agclose_t *agclose;
  agnode_t *agnode;
  agset_t *agset;
  agedge_t *agedge;
  Agdesc_t Agdirected;

private:
  cgraph_t();
};

} // namespace pivot
