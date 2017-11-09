// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/HTMLMenuItemElement.h"

#include "core/HTMLNames.h"
#include "core/dom/ElementTraversal.h"
#include "core/events/Event.h"
#include "core/frame/UseCounter.h"
#include "core/html/parser/HTMLParserIdioms.h"

namespace blink {

using namespace HTMLNames;

inline HTMLMenuItemElement::HTMLMenuItemElement(Document& document)
    : HTMLElement(HTMLNames::menuitemTag, document) {
}

bool HTMLMenuItemElement::IsURLAttribute(const Attribute& attribute) const {
  return HTMLElement::IsURLAttribute(attribute);
}

void HTMLMenuItemElement::ParseAttribute(
    const AttributeModificationParams& params) {
  HTMLElement::ParseAttribute(params);
}

void HTMLMenuItemElement::DefaultEventHandler(Event* event) {
  if (event->type() == EventTypeNames::click) {
    if (DeprecatedEqualIgnoringCase(FastGetAttribute(typeAttr), "checkbox")) {
      if (FastHasAttribute(checkedAttr))
        removeAttribute(checkedAttr);
      else
        setAttribute(checkedAttr, "checked");
    } 
    event->SetDefaultHandled();
  }
}

String HTMLMenuItemElement::label() const {
  const AtomicString label = FastGetAttribute(labelAttr);
  if (!label.IsNull())
    return label;
  return conceptualLabel();
}

void HTMLMenuItemElement::setLabel(const AtomicString& label) {
  setAttribute(labelAttr, label);
}

String HTMLMenuItemElement::conceptualLabel() const {
  const AtomicString label = FastGetAttribute(labelAttr);
  if (!label.IsEmpty())
    return label;
  return this->textContent(false)
      .StripWhiteSpace(IsHTMLSpace<UChar>)
      .SimplifyWhiteSpace(IsHTMLSpace<UChar>);
}

DEFINE_NODE_FACTORY(HTMLMenuItemElement)

}  // namespace blink