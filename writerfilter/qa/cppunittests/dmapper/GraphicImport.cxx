/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/GraphicImport.cxx.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

char const DATA_DIRECTORY[] = "/writerfilter/qa/cppunittests/dmapper/data/";

CPPUNIT_TEST_FIXTURE(Test, testGroupShapeRotation)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "group-shape-rotation.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nVertPosition = 0;
    xShape->getPropertyValue("VertOrientPosition") >>= nVertPosition;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1221
    // - Actual  : -2048
    // i.e. the group shape had a so low vertical position that the line shape did not point into
    // it.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1221), nVertPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawShapeInlineEffect)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "draw-shape-inline-effect.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nBottomMargin = 0;
    xShape->getPropertyValue("BottomMargin") >>= nBottomMargin;
    // 273 in mm100 is 98425 EMUs from the file.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 273
    // - Actual  : 0
    // i.e. the layout result had less pages than expected (compared to Word).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(273), nBottomMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testInlineAnchoredZOrder)
{
    // Load a document which has two shapes: an inline one and an anchored one. The inline has no
    // explicit ZOrder, the anchored one has, and it's set to a value so it's visible.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "inline-anchored-zorder.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<container::XNamed> xOval(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Oval 2
    // - Actual  :
    // i.e. the rectangle (with no name) was on top of the oval one, not the other way around.
    CPPUNIT_ASSERT_EQUAL(OUString("Oval 2"), xOval->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testInlineInShapeAnchoredZOrder)
{
    // This document has a textbox shape and then an inline shape inside that.
    // The ZOrder of the inline shape is larger than the hosting textbox, so the image is visible.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "inline-inshape-anchored-zorder.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<container::XNamed> xOval(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Picture 1
    // - Actual  : Text Box 2
    // i.e. the image was behind the textbox that was hosting it.
    CPPUNIT_ASSERT_EQUAL(OUString("Picture 1"), xOval->getName());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
