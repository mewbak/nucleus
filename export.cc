#include "bb.h"
#include "edge.h"
#include "insn.h"
#include "cfg.h"
#include "log.h"
#include "nucleus.h"
#include "export.h"


int
export_bin2ida(std::string &fname, Binary *bin, std::list<DisasmSection> *disasm, CFG *cfg)
{
  FILE *f;
  uint64_t entry;
  size_t i;

  f = fopen(fname.c_str(), "w");
  if(!f) {
    print_err("cannot open file '%s' for writing", fname.c_str());
    return -1;
  }

  fprintf(f, "\"\"\"\n");
  fprintf(f, "Script generated by %s\n", NUCLEUS_VERSION);
  fprintf(f, "\"\"\"\n");
  fprintf(f, "\n");
  fprintf(f, "import idaapi\n");
  fprintf(f, "import idautils\n");
  fprintf(f, "import idc\n");
  fprintf(f, "\n");
  fprintf(f, "idaapi.autoWait()\n");
  fprintf(f, "\n");
  fprintf(f, "def mark_functions():\n");
  fprintf(f, "    functions = [\n");
  i = 0;
  for(auto &func: cfg->functions) {
    if(func.entry.empty()) continue;
    entry = func.entry.front()->start;
    if(!(i % 5)) fprintf(f, "        ");
    fprintf(f, "0x%jx, ", entry);
    if(!(++i % 5)) fprintf(f, "\n");
  }
  fprintf(f, "    ]\n");
  fprintf(f, "    for seg in idautils.Segments():\n");
  fprintf(f, "        if idaapi.segtype(idc.SegStart(seg)) != idaapi.SEG_CODE:\n");
  fprintf(f, "            continue\n");
  fprintf(f, "        for f in idautils.Functions(idc.SegStart(seg), idc.SegEnd(seg)):\n");
  fprintf(f, "            print 'nucleus: deleting function 0x%%x' %% (f)\n");
  fprintf(f, "            idc.DelFunction(f)\n");
  fprintf(f, "    for f in functions:\n");
  fprintf(f, "        print 'nucleus: defining function 0x%%x' %% (f)\n");
  fprintf(f, "        if idc.MakeCode(f):\n");
  fprintf(f, "            idc.MakeFunction(f)\n");
  fprintf(f, "\n");
  fprintf(f, "mark_functions()\n");

  fclose(f);

  return 0;
}


int
export_cfg2dot(std::string &fname, CFG *cfg)
{
  FILE *f;
  BB *bb;

  f = fopen(fname.c_str(), "w");
  if(!f) {
    print_err("cannot open file '%s' for writing", fname.c_str());
    return -1;
  }

  fprintf(f, "digraph G {\n\n");
  for(auto &kv : cfg->start2bb) {
    bb = kv.second;
    for(auto &e : bb->targets) {
      fprintf(f, "bb_%jx -> bb_%jx [ label=\"%s\" ];\n", e.src->start, e.dst->start, e.type2str().c_str());
    }
  }
  fprintf(f, "}\n");

  fclose(f);

  return 0;
}

