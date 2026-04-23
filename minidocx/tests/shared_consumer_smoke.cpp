#include "minidocx/minidocx.hpp"

int main() {
  md::Document doc;
  auto section = doc.addSection();
  section->addParagraph()->addRichText("shared-consumer-smoke");
  return 0;
}
