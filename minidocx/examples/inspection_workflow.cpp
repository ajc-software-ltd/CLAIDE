#include "minidocx/minidocx.hpp"

#include <iostream>
#include <string>

int main()
{
  using namespace md;

  try {
    Document doc;
    auto section = doc.addSection();
    auto paragraph = section->addParagraph();
    paragraph->addRichText("Initial text");

    const auto statsBefore = inspection::summarize(doc);
    std::cout << "Before: sections=" << statsBefore.sectionCount
              << ", paragraphs=" << statsBefore.paragraphCount
              << ", runs=" << statsBefore.runCount << '\n';

    editing::CommandResult result = editing::applyCommand(
        doc, editing::ReplaceParagraphTextCommand{{0, 0, 0, false}, "Updated from command API"});
    if (!result.success) {
      std::cerr << "Command failed: " << result.message << '\n';
      return 1;
    }

    const std::string visibleText = inspection::extractVisibleText(doc);
    std::cout << "Visible text: " << visibleText << '\n';

    const auto layout = inspection::buildLayout(doc);
    std::cout << "Layout pages: " << layout.pages.size() << '\n';

    doc.saveAs("out/inspection_workflow.docx");
    std::cout << "Saved: out/inspection_workflow.docx" << '\n';
  }
  catch (const Exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
