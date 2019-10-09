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
	fFindReplaceHandler = new FindReplaceHandler(fEditor, fWindow);
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

TEST_F(FindReplaceTest, BackwardsFindAfterRegularFindFindsPreviousPhrase)
{
	fEditor->LockLooper();
	// just after second ipsum
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

TEST_F(FindReplaceTest, RegularFindAfterBackwardsFindFindsNextPhrase)
{
	fEditor->LockLooper();
	// just after first ipsum
	fEditor->SendMessage(SCI_GOTOPOS, 11);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	message.AddBool("backwards", true);
	fMessenger->SendMessage(&message, &reply);

	message.RemoveName("backwards");
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 118);
	EXPECT_EQ(current, 113);
}

TEST_F(FindReplaceTest, FirstNoResultFoundSecondWrapAroundEnabledFindsTheResult)
{
	fEditor->LockLooper();
	// just after last ipsum
	fEditor->SendMessage(SCI_GOTOPOS, 370);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	fMessenger->SendMessage(&message, &reply);

	bool found = reply.GetBool("found", true);
	EXPECT_EQ(found, false);

	message.AddBool("wrapAround", true);
	fMessenger->SendMessage(&message, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 6);
	EXPECT_EQ(current, 11);
}

TEST_F(FindReplaceTest, NoResultDoesntChangeSelection)
{
	fEditor->LockLooper();
	// just after last ipsum
	fEditor->SendMessage(SCI_GOTOPOS, 370);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage message(FindReplaceHandler::FIND);
	message.AddString("findText", "ipsum");
	fMessenger->SendMessage(&message, &reply);

	bool found = reply.GetBool("found", true);
	EXPECT_EQ(found, false);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	EXPECT_EQ(anchor, 370);
	EXPECT_EQ(current, 370);
}


TEST_F(FindReplaceTest, InSelectionFindsOnlyInSelection)
{
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

TEST_F(FindReplaceTest, ReplacesAllOccurences)
{
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 100);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage replaceMessage(FindReplaceHandler::REPLACEALL);
	replaceMessage.AddString("findText", "ipsum");
	replaceMessage.AddString("replaceText", "muspimus");
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	int32 replaced = reply.GetInt32("replaced", 0);
	EXPECT_EQ(replaced, 3);

	// before replacement caret was after first ipsum
	// after replacement it should be shifted
	EXPECT_EQ(anchor, 100 + 3);
	EXPECT_EQ(current, 100 + 3);

	std::string text(8, '\0');
	fEditor->GetText(6, 8, text.data());
	EXPECT_EQ(text, "muspimus");

	fEditor->GetText(113 + 3, 8, text.data());
	EXPECT_EQ(text, "muspimus");

	fEditor->GetText(365 + 3 + 3, 8, text.data());
	EXPECT_EQ(text, "muspimus");
}

TEST_F(FindReplaceTest, ReplacesAllOccurencesRecursiveCase)
{
	fEditor->LockLooper();
	fEditor->SetText("ipsum ipsum\nipsum");
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage replaceMessage(FindReplaceHandler::REPLACEALL);
	replaceMessage.AddString("findText", "ipsum");
	replaceMessage.AddString("replaceText", "ipsum ipsum");
	fMessenger->SendMessage(&replaceMessage, &reply);

	int32 replaced = reply.GetInt32("replaced", 0);
	EXPECT_EQ(replaced, 3);

	const int length = fEditor->SendMessage(SCI_GETLENGTH);
	std::string text(length, '\0');
	fEditor->GetText(0, length + 1, text.data());
	EXPECT_EQ(text, "ipsum ipsum ipsum ipsum\nipsum ipsum");
}

TEST_F(FindReplaceTest, ReplacesAllOccurencesOnlyInSelection)
{
	fEditor->LockLooper();
	// before second ipsum
	fEditor->SendMessage(SCI_SETSELECTIONSTART, 110);
	// after last ipsum
	fEditor->SendMessage(SCI_SETSELECTIONEND, 375);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage replaceMessage(FindReplaceHandler::REPLACEALL);
	replaceMessage.AddString("findText", "ipsum");
	replaceMessage.AddString("replaceText", "muspimus");
	replaceMessage.AddBool("inSelection", true);
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	int32 replaced = reply.GetInt32("replaced", 0);
	EXPECT_EQ(replaced, 2);

	// after replacement selection should be expanded
	EXPECT_EQ(anchor, 110);
	EXPECT_EQ(current, 375 + 3 + 3);

	std::string text(5, '\0');
	fEditor->GetText(6, 5, text.data());
	EXPECT_EQ(text, "ipsum");

	text.resize(8);
	fEditor->GetText(113, 8, text.data());
	EXPECT_EQ(text, "muspimus");

	fEditor->GetText(365 + 3, 8, text.data());
	EXPECT_EQ(text, "muspimus");
}

TEST_F(FindReplaceTest, ReplaceAllDoesntGoIntoInfiniteLoop)
{
	fEditor->LockLooper();
	fEditor->SetText("ipsum ipsum\nipsum");
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage replaceMessage(FindReplaceHandler::REPLACEALL);
	replaceMessage.AddString("findText", "ipsum");
	replaceMessage.AddString("replaceText", "ipsum ipsum");
	fMessenger->SendMessage(&replaceMessage, &reply);
}

TEST_F(FindReplaceTest, ReplaceFindReplacesAndGoesToNextResult)
{
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "ipsum");
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACEFIND);
	replaceMessage.AddString("replaceText", "muspi");
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	std::string text(5, '\0');
	fEditor->GetText(6, 5, text.data());
	EXPECT_EQ(text, "muspi");

	EXPECT_EQ(anchor, 113);
	EXPECT_EQ(current, 118);
}

TEST_F(FindReplaceTest, BackwardsReplaceFindReplacesAndGoesToNextResult)
{
	fEditor->LockLooper();
	fEditor->SendMessage(SCI_GOTOPOS, 350);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "ipsum");
	findMessage.AddBool("backwards", true);
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACEFIND);
	replaceMessage.AddString("replaceText", "muspi");
	fMessenger->SendMessage(&replaceMessage, &reply);

	const Sci_Position anchor = fEditor->SendMessage(SCI_GETANCHOR);
	const Sci_Position current = fEditor->SendMessage(SCI_GETCURRENTPOS);

	std::string text(5, '\0');
	fEditor->GetText(113, 5, text.data());
	EXPECT_EQ(text, "muspi");

	EXPECT_EQ(anchor, 11);
	EXPECT_EQ(current, 6);
}


TEST_F(FindReplaceTest, Issue95ReplacedTextIsNotFoundRecursively)
/*
Issue 95 description:
Currently, if I replace "span" with "span class" the cursor is placed in front
of "span class". If I click "Find" now, the "span" of the just inserted "span
class" is found again and replacing would result in "span span class".
*/
{
	fEditor->LockLooper();
	fEditor->SetText("<span test>");
	fEditor->SendMessage(SCI_GOTOPOS, 0);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "span");
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACE);
	replaceMessage.AddString("replaceText", "span class");
	fMessenger->SendMessage(&replaceMessage, &reply);

	findMessage.AddString("findText", "span");
	fMessenger->SendMessage(&findMessage, &reply);
	bool found = reply.GetBool("found", true);
	EXPECT_EQ(found, false);
}

TEST_F(FindReplaceTest, Issue95ReplacedTextIsNotFoundRecursivelyBackwards)
{
	fEditor->LockLooper();
	fEditor->SetText("<span test>");
	fEditor->SendMessage(SCI_GOTOPOS, 11);
	fEditor->UnlockLooper();

	BMessage reply;
	BMessage findMessage(FindReplaceHandler::FIND);
	findMessage.AddString("findText", "span");
	findMessage.AddBool("backwards", true);
	fMessenger->SendMessage(&findMessage, &reply);

	BMessage replaceMessage(FindReplaceHandler::REPLACE);
	replaceMessage.AddString("replaceText", "span class");
	fMessenger->SendMessage(&replaceMessage, &reply);

	findMessage.AddString("findText", "span");
	findMessage.AddBool("backwards", true);
	fMessenger->SendMessage(&findMessage, &reply);
	bool found = reply.GetBool("found", true);
	EXPECT_EQ(found, false);
}

// TODO: Find then find in selection searches in last selection not result
// TODO: In selection Replace & Find
