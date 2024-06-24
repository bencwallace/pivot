#include "walk_node.h"

namespace pivot {

template <int Dim> void walk_node<Dim>::todot(const std::string &path) const {
  gvc_t &gvc = gvc_t::load();
  cgraph_t &cgraph = cgraph_t::load();

  GVC_t *context = gvc.gvContext();
  Agraph_t *g = cgraph.agopen((char *)"G", cgraph.Agdirected, nullptr);
  cgraph.agattr(g, AGNODE, (char *)"shape", (char *)"circle");

  todot(g, cgraph);

  gvc.gvLayout(context, g, "dot");
  gvc.gvRenderFilename(context, g, "dot", path.c_str());

  gvc.gvFreeLayout(context, g);
  cgraph.agclose(g);
  gvc.gvFreeContext(context);
}

template <int Dim> Agnode_t *walk_node<Dim>::todot(Agraph_t *g, const cgraph_t &cgraph) const {
  auto name = std::to_string(id_);
  Agnode_t *node = cgraph.agnode(g, (char *)name.c_str(), 1);
  if (cgraph.agset((void *)node, (char *)"shape", (char *)"box") != 0) {
    throw std::runtime_error("failed to set shape");
  }

  auto label = "id: " + std::to_string(id_) + "\\l";
  label += "num_sites: " + std::to_string(num_sites_) + "\\l";
  label += "symm: " + symm_.to_string() + "\\l";
  label += "box: " + bbox_.to_string() + "\\l";
  label += "end: " + end_.to_string() + "\\l";
  cgraph.agset(node, (char *)"label", (char *)label.c_str());

  if (left_ != nullptr) {
    Agnode_t *left_node;
    if (left_->is_leaf()) {
      left_node = cgraph.agnode(g, (char *)(name + "L").c_str(), 1);
      cgraph.agset(left_node, (char *)"label", (char *)std::to_string(id_ - 1).c_str());
    } else {
      left_node = left_->todot(g, cgraph);
    }
    cgraph.agedge(g, node, left_node, nullptr, 1);
  }
  if (right_ != nullptr) {
    Agnode_t *right_node;
    if (right_->is_leaf()) {
      right_node = cgraph.agnode(g, (char *)(name + "R").c_str(), 1);
      cgraph.agset(right_node, (char *)"label", (char *)std::to_string(id_).c_str());
    } else {
      right_node = right_->todot(g, cgraph);
    }
    cgraph.agedge(g, node, right_node, nullptr, 1);
  }
  return node;
}

} // namespace pivot
