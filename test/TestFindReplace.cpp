/*
 * Copyright 2019 Kacper Kasper <kacperkasper@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include <gtest/gtest.h>

#include <Application.h>
#include <GroupLayout.h>
#include <ScintillaView.h>
#include <Window.h>

#include "editor/FindReplaceHandler.h"


class FindReplaceTest : public ::testing::Test
{
protected:
	BApplication* fApplication;
	BWindow* fWindow;
	BScintillaView* fEditor;
	FindReplaceHandler* fFindReplaceHandler;
	BMessenger* fMessenger;

	void SetUp() override;
	void TearDown() override;
};


void
FindReplaceTest::SetUp()
{
	fApplication = new BApplication("application/x-vnd.KapiX-KoderFindReplaceTest");
	fWindow = new BWindow(BRect(100, 100, 400, 400), "FindReplaceTest", B_DOCUMENT_WINDOW, 0);
	fEditor = new BScintillaView("EditorView", 0, true, true, B_NO_BORDER);
	fEditor->SetText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
		"Duis congue aliquam eros eget rhoncus. Mauris ultricies ipsum urna, "
		"vitae scelerisque lacus facilisis molestie. Curabitur pulvinar "
		"imperdiet nibh ut lacinia. Donec a dictum sapien, id rhoncus ex. "
		"Integer et lorem malesuada, sagittis massa nec, imperdiet ante. "
		"Pellentesque quis lectus dolor. Vestibulum ante ipsum primis in "
		"faucibus orci luctus et ultrices posuere cubilia Curae; Cras ac purus "
		"auctor, mattis risus eu, ultricies lectus.");
	fFindReplaceHandler = new FindReplaceHandler(fEditor);
	fWindow->AddHandler(fFindReplaceHandler);
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL, 0);
	fWindow->SetLayout(layout);
	fWindow->Show();
	layout->AddView(fEditor);
	fMessenger = new BMessenger(fFindReplaceHandler, fWindow);
}


void
FindReplaceTest::TearDown()
{
	delete fMessenger;
	fApplication->PostMessage(B_QUIT_REQUESTED);
	delete fApplication;
	delete fFindReplaceHandler;
}

TEST_F(FindReplaceTest, FindsSpecifiedPhraseAndPlacesCursorAtTheEnd)
{
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	fMessenger->SendMessage(&message, &reply);

	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 6);
	EXPECT_EQ(current, 11);
}

TEST_F(FindReplaceTest, SubsequentFindFindsNextPhrase)
{
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	fMessenger->SendMessage(&message, &reply);
	fMessenger->SendMessage(&message, &reply);

	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 113);
	EXPECT_EQ(current, 118);
}

TEST_F(FindReplaceTest, BackwardsFindsPreviousPhraseAndPlacesCursorAtTheBeginning)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, length);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	message.AddBool("backwards", true);

	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 370);
	EXPECT_EQ(current, 365);
}

TEST_F(FindReplaceTest, SubsequentBackwardsFindFindsPreviousPhrase)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, length);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	message.AddBool("backwards", true);

	fMessenger->SendMessage(&message, &reply);
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 118);
	EXPECT_EQ(current, 113);
}

TEST_F(FindReplaceTest, BackwardsAfterRegularFindFindsPreviousPhrase)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	// just after first ipsum
	fEditor->SendMessage(SCI_GOTOPOS, 118);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	fMessenger->SendMessage(&message, &reply);

	message.AddBool("backwards", true);
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 118);
	EXPECT_EQ(current, 113);
}

TEST_F(FindReplaceTest, FirstNoResultFoundSecondWrapAroundEnabledFindsTheResult)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	// just after last ipsum
	fEditor->SendMessage(SCI_GOTOPOS, 370);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	fMessenger->SendMessage(&message, &reply);

	message.AddBool("wrapAround", true);
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 6);
	EXPECT_EQ(current, 11);
}

TEST_F(FindReplaceTest, InSelectionFindsOnlyInSelection)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	// before second ipsum
	fEditor->SendMessage(SCI_SETSELECTIONSTART, 110);
	// after last ipsum
	fEditor->SendMessage(SCI_SETSELECTIONEND, 375);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	message.AddBool("inSelection", true);
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 113);
	EXPECT_EQ(current, 118);
}

TEST_F(FindReplaceTest, InSelectionWrapsAroundOnlyInSelection)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	// before second ipsum
	fEditor->SendMessage(SCI_SETSELECTIONSTART, 110);
	// after last ipsum
	fEditor->SendMessage(SCI_SETSELECTIONEND, 375);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	message.AddBool("inSelection", true);
	message.AddBool("wrapAround", true);
	fMessenger->SendMessage(&message, &reply);
	fMessenger->SendMessage(&message, &reply);
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 113);
	EXPECT_EQ(current, 118);
}

TEST_F(FindReplaceTest, ReplaceReplacesOnlyOnePhrase)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "ipsum");
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACE);
	replaceMessage.AddString("replaceText", "muspi");
	fMessenger->SendMessage(&replaceMessage, &reply);

	std::string text(5, '\0');
	fEditor->GetText(6, 5, text.data());
	EXPECT_EQ(text, "muspi");

	fEditor->GetText(113, 5, text.data());
	EXPECT_EQ(text, "ipsum");
}

TEST_F(FindReplaceTest, ReplaceSelectsReplacement)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "ipsum");
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACE);
	replaceMessage.AddString("replaceText", "muspi");
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 6);
	EXPECT_EQ(current, 11);
}

TEST_F(FindReplaceTest, ReplaceSelectsLongerReplacement)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "ipsum");
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACE);
	replaceMessage.AddString("replaceText", "muspimus");
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 6);
	EXPECT_EQ(current, 14);
}

TEST_F(FindReplaceTest, ReplaceSwapsAnchorAndCaretAfterBackwardsSearch)
{
	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	fEditor->LockLooper();
	// just after first ipsum
	fEditor->SendMessage(SCI_GOTOPOS, 100);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "ipsum");
	findMessage.AddBool("backwards", true);
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACE);
	replaceMessage.AddString("replaceText", "muspimus");
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 14);
	EXPECT_EQ(current, 6);
}
