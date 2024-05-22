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
  using agopen_t = Agraph_t *(*)(char *, Agdesc_t, Agdisc_t *);
  using agattr_t = Agsym_t *(*)(Agraph_t *, int, char *, char *);
  using agclose_t = int (*)(Agraph_t *);
  using agnode_t = Agnode_t *(*)(Agraph_t *, char *, int);
  using agset_t = int (*)(void *, char *, char *);
  using agedge_t = Agedge_t *(*)(Agraph_t *, Agnode_t *, Agnode_t *, char *, int);

  static cgraph_t &load();

  agopen_t agopen;
  agattr_t agattr;
  agclose_t agclose;
  agnode_t agnode;
  agset_t agset;
  agedge_t agedge;
  Agdesc_t Agdirected;

private:
  cgraph_t();
};

} // namespace pivot
