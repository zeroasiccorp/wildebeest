// Helpers that let the plugin build against every supported Yosys version.

#ifndef WILDEBEEST_YOSYS_COMPAT_H
#define WILDEBEEST_YOSYS_COMPAT_H

#include "kernel/yosys.h"

// Yosys 0.71 replaced the global IdString table with one TwinePool per
// Design. Since then ID(x) only accepts the names Yosys declares in
// kernel/constids.inc, a bare IdString can't be turned back into a string
// without its design's pool, and log_id() is gone.
#if YOSYS_MAJOR > 0 || YOSYS_MINOR >= 71
#define WB_YOSYS_PER_DESIGN_IDS 1
#endif

// WB_ID(owner, name) is the IdString for "\name" when name is not a Yosys
// builtin id, such as a wildebeest or vendor cell, port or parameter name.
// owner is a Design, Module, Cell or Wire pointer from the design the id is
// used in. Before Yosys 0.71 it is ignored and this is plain ID(name).
#ifdef WB_YOSYS_PER_DESIGN_IDS
#define WB_ID(owner, id) ((owner)->twines().add("\\" #id))
#else
#define WB_ID(owner, id) ID(id)
#endif

YOSYS_NAMESPACE_BEGIN

#ifdef WB_YOSYS_PER_DESIGN_IDS
// Stand-ins for the log_id() overloads removed in Yosys 0.71, for named
// objects (Cell, Wire, Module, ...) and for their name and type fields. They
// return std::string, which the format-checked log() and stringf() of these
// versions accept for %s.
template <typename T>
inline auto log_id(const T *obj, const char *nullstr = nullptr)
    -> decltype(obj->name.unescape()) {
  if (nullstr && obj == nullptr)
    return nullstr;
  return obj->name.unescape();
}

template <typename D>
inline std::string log_id(const RTLIL::WrappedIdBase<D> &id) {
  return id.unescape();
}
#endif

// log_id() of a bare IdString, which from Yosys 0.71 on needs the pool of
// the design it belongs to. owner is a Design, Module, Cell or Wire pointer
// from that design.
template <typename T>
inline auto wb_log_id(const T *owner, RTLIL::IdString id) {
#ifdef WB_YOSYS_PER_DESIGN_IDS
  return owner->twines().unescaped_str(id);
#else
  (void)owner;
  return log_id(id);
#endif
}

YOSYS_NAMESPACE_END

#endif
