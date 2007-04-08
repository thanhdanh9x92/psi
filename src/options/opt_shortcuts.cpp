/*
 *
 * opt_shortcuts.cpp - an OptionsTab for setting the Keyboard Shortcuts of Psi
 * Copyright (C) 2006 Cestonaro Thilo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "opt_shortcuts.h"
#include "psioptions.h"
#include "shortcutmanager.h"
#include "grepshortcutkeydlg.h"

#include "ui_opt_shortcuts.h"

#define ITEMKIND Qt::UserRole
#define OPTIONSTREEPATH Qt::UserRole + 1

/**
 * \class the Ui for the Options Tab Shortcuts
 */
class OptShortcutsUI : public QWidget, public Ui::OptShortcuts
{
public:
	OptShortcutsUI() : QWidget() { setupUi(this); }
};

//----------------------------------------------------------------------------
// OptionsTabShortcuts
//----------------------------------------------------------------------------

/**
 * \brief Constructor of the Options Tab Shortcuts Class
 */
OptionsTabShortcuts::OptionsTabShortcuts(QObject *parent)
: OptionsTab(parent, "shortcuts", "", tr("Shortcuts"), tr("Options for Psi Shortcuts"), "psi/shortcuts")
{
	w = 0;
}

/**
 * \brief Destructor of the Options Tab Shortcuts Class
 */
OptionsTabShortcuts::~OptionsTabShortcuts()
{
}

/**
 * \brief widget, creates the Options Tab Shortcuts Widget
 * \return QWidget*, points to the previously created widget
 */
QWidget *OptionsTabShortcuts::widget()
{
	if ( w )
		return 0;

	w = new OptShortcutsUI();
	OptShortcutsUI *d = (OptShortcutsUI *)w;
	
	d->treeShortcuts->setColumnWidth(0, 320);	

	PsiOptions *options = PsiOptions::instance();
	QTreeWidgetItem *topLevelItem;
	QList<QString> shortcutGroups = options->getChildOptionNames("options.shortcuts", true, true);

	/* step through the shortcut groups e.g. chatdlg */
	foreach(QString shortcutGroup, shortcutGroups) {
		topLevelItem = new QTreeWidgetItem(d->treeShortcuts);
		topLevelItem->setText(0, options->getComment(shortcutGroup));
		topLevelItem->setData(0, OPTIONSTREEPATH, QVariant(shortcutGroup));
		topLevelItem->setData(0, ITEMKIND, QVariant((int)OptionsTabShortcuts::TopLevelItem));
		topLevelItem->setExpanded(true);
		d->treeShortcuts->addTopLevelItem(topLevelItem);
	}

	d->add->setEnabled(false);
	d->remove->setEnabled(false);

	connect(d->treeShortcuts, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
	connect(d->treeShortcuts, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem *, int)));
	connect(d->add, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(d->remove, SIGNAL(clicked()), this, SLOT(onRemove()));
	return w;
}

/**
 * \brief	applyOptions, if options have changed, they will be applied by calling this function
 * \param	opt, unused, totally ignored
 */
void OptionsTabShortcuts::applyOptions(Options *opt) {
	Q_UNUSED(opt);
	if ( !w )
		return;

	OptShortcutsUI *d = (OptShortcutsUI *)w;
	PsiOptions *options = PsiOptions::instance();

	int toplevelItemsCount = d->treeShortcuts->topLevelItemCount();
	int shortcutItemsCount;
	int keyItemsCount;
	QTreeWidgetItem *topLevelItem;
	QTreeWidgetItem *shortcutItem;
	QTreeWidgetItem *keyItem;
	QString optionsPath;
	QString comment;
	QList<QString> children;
	QList<QKeySequence> keys;

	/* step through the Toplevel Items */
	for(int topLevelIndex = 0 ; topLevelIndex < toplevelItemsCount; topLevelIndex++) {
		topLevelItem = d->treeShortcuts->topLevelItem(topLevelIndex);
		shortcutItemsCount = topLevelItem->childCount();

		/* step through the Shortcut Items */
		for(int shortcutItemIndex = 0; shortcutItemIndex < shortcutItemsCount; shortcutItemIndex++) {
			shortcutItem = topLevelItem->child(shortcutItemIndex);
			keyItemsCount = shortcutItem->childCount();
			
			/* get the Options Path of the Shortcut Item */
			optionsPath = shortcutItem->data(0, OPTIONSTREEPATH).toString();

			/* just one Key Sequence */
			if(keyItemsCount == 1) {
				/* so set the option to this keysequence directly */
				keyItem = shortcutItem->child(0);
				options->setOption(optionsPath, QVariant(keyItem->text(1)));
			}
			else if(keyItemsCount > 1){
				/* more than one, then collect them in a list */
				QList<QVariant> keySequences;
				for(int keyItemIndex = 0; keyItemIndex < keyItemsCount; keyItemIndex++) {
					keyItem = shortcutItem->child(keyItemIndex);
					keySequences.append(QVariant(keyItem->text(1)));
				}
				
				options->setOption(optionsPath, QVariant(keySequences));
			}
			else {
				/* zero key sequences, so set an empty variant, so it will be written empty to the options.xml */
				options->setOption(optionsPath, QVariant());
			}
		}
	}
}

/**
 * \brief	restoreOptions, reads in the currently set options
 * \param	opt, unused, totally ignored
 */
void OptionsTabShortcuts::restoreOptions(const Options *opt)
{
	Q_UNUSED(opt);

	if ( !w )
		return;

	OptShortcutsUI *d = (OptShortcutsUI *)w;
	PsiOptions *options = PsiOptions::instance();

	int toplevelItemsCount = d->treeShortcuts->topLevelItemCount();
	QTreeWidgetItem *topLevelItem;
	QTreeWidgetItem *shortcutItem;
	QTreeWidgetItem *keyItem;
	QString optionsPath;
	QString comment;
	QList<QString> shortcuts;
	QList<QKeySequence> keys;
	int keyItemsCount;

	/* step through the toplevel items */
	for(int toplevelItemIndex = 0 ; toplevelItemIndex < toplevelItemsCount; toplevelItemIndex++) {
		topLevelItem = d->treeShortcuts->topLevelItem(toplevelItemIndex);
		optionsPath = topLevelItem->data(0, OPTIONSTREEPATH).toString();

		/* if a optionsPath was saved in the toplevel item, we can get the shortcuts and the keys for the shortcuts */
		if(!optionsPath.isEmpty()) {
			
			shortcuts = options->getChildOptionNames(optionsPath, true, true);
			/* step through the shortcuts */
			foreach(QString shortcut, shortcuts) {
				
				keys = ShortcutManager::instance()->shortcuts(shortcut.mid(QString("options.shortcuts").length() + 1));
				comment = options->getComment(shortcut);
				
				/* create the TreeWidgetItem and set the Data the Kind and it's Optionspath and append it */
				shortcutItem = new QTreeWidgetItem(topLevelItem);
				shortcutItem->setText(0, comment);
				shortcutItem->setData(0, ITEMKIND, QVariant((int)OptionsTabShortcuts::ShortcutItem));
				shortcutItem->setData(0, OPTIONSTREEPATH, QVariant(shortcut));
				topLevelItem->addChild(shortcutItem);

				/* step through this shortcut's keys and create 'Key XXXX' entries for them */
				keyItemsCount = 1;
				foreach(QKeySequence key, keys) {
					keyItem = new QTreeWidgetItem(shortcutItem);
					keyItem->setText(0, QString("Key %1").arg(keyItemsCount++));
					keyItem->setData(0, ITEMKIND, QVariant((int)OptionsTabShortcuts::KeyItem));
					keyItem->setText(1, key.toString());
					shortcutItem->addChild(keyItem);
				}
			}
		}
	}
}

/**
 * \brief	Button Add pressed, creates a new Key entry and calls onItemDoubleClicked
 */
void OptionsTabShortcuts::onAdd() {
	OptShortcutsUI *d = (OptShortcutsUI *)w;
	
	QTreeWidgetItem	*shortcutItem;
	QTreeWidgetItem *newKeyItem;

	QList<QTreeWidgetItem *> selectedItems = d->treeShortcuts->selectedItems();
	QString	optionsPath;
	Kind itemKind;

	if(selectedItems.count() == 0)
		return;

	shortcutItem = selectedItems[0];
	itemKind = (Kind)shortcutItem->data(0, ITEMKIND).toInt();

	switch(itemKind) {
		case OptionsTabShortcuts::KeyItem:
			/* it was a keyItem, so get it's parent */
			shortcutItem = shortcutItem->parent();
			break;	
		case OptionsTabShortcuts::ShortcutItem:
			break;
		default:
			return;
	}

	/* so the user can see that we have create something there */
	shortcutItem->setExpanded(true);
	
	newKeyItem = new QTreeWidgetItem(shortcutItem);
	newKeyItem->setText(0, QString("Key %1").arg(shortcutItem->childCount()));
	newKeyItem->setData(0, ITEMKIND, QVariant(OptionsTabShortcuts::KeyItem));
	shortcutItem->addChild(newKeyItem);

	/* unselect ANY selected item */
	for(int selectedItemIndex=0; selectedItemIndex < selectedItems.count(); selectedItemIndex++)
		selectedItems[selectedItemIndex]->setSelected(false);

	/* select the new one, so the simulated Double click goes to this item */
	newKeyItem->setSelected(true);
	onItemDoubleClicked(newKeyItem, 0);
}

/**
 * \brief Button Remove pressed, removes the currently selected item, if it is a Keyitem and it's just one selected
 */
void OptionsTabShortcuts::onRemove() {
	OptShortcutsUI *d = (OptShortcutsUI *)w;
	QList<QTreeWidgetItem *> selectedItems = d->treeShortcuts->selectedItems();
	
	/* zero or more then one selected Item(s), so we can't add or remove anything, disable the buttons */
	if(selectedItems.count() == 0 || selectedItems.count() > 1) {
		d->add->setEnabled(false);
		d->remove->setEnabled(false);
		return;
	}

	QTreeWidgetItem	*shortcutItem;
	QTreeWidgetItem	*keyItem;
	int keyItemsCount;
	
	keyItem = selectedItems[0];

	/* we need a Item with the Kind "KeyItem", else we could / should not remove it */
	if((Kind)keyItem->data(0, ITEMKIND).toInt() == OptionsTabShortcuts::KeyItem) {
		shortcutItem = keyItem->parent();
		/* remove the key item from the shortcut item's children */
		shortcutItem->takeChild(shortcutItem->indexOfChild(keyItem));

		/* rename the children which are left over */
		keyItemsCount = shortcutItem->childCount();
		for( int keyItemIndex = 0; keyItemIndex < keyItemsCount; keyItemIndex++)
			shortcutItem->child(keyItemIndex)->setText(0, QString("Key %1").arg(keyItemIndex + 1));

		/* notify the options dlg that data was changed */
		emit dataChanged();	
	}
}

/**
 * \brief	in the treeview, the selected item has changed, the add and remove buttons are
 * 			enabled or disabled, depening on the selected item type
 */
void OptionsTabShortcuts::onItemSelectionChanged() {
	OptShortcutsUI *d = (OptShortcutsUI *)w;
	QList<QTreeWidgetItem *> selectedItems = d->treeShortcuts->selectedItems();
	Kind itemKind;

	/* zero or more then one selected Item(s), so we can't add or remove anything, disable the buttons */
	if(selectedItems.count() == 0 || selectedItems.count() > 1) {
		d->add->setEnabled(false);
		d->remove->setEnabled(false);
		return;
	}

	itemKind = (Kind)selectedItems[0]->data(0, ITEMKIND).toInt();
	switch(itemKind) {
		case OptionsTabShortcuts::TopLevelItem:
			/* for a topLevel Item, we can't do anything neither add a key, nor remove one */
			d->add->setEnabled(false);
			d->remove->setEnabled(false);
			break;
		case OptionsTabShortcuts::ShortcutItem:
			/* at a shortcut Item, we can add a key, but not remove it */
			d->add->setEnabled(true);
			d->remove->setEnabled(false);
			break;
		case OptionsTabShortcuts::KeyItem:
			/* at a key item, we can add a key to it's parent shortcut item, or remove it */
			d->add->setEnabled(true);
			d->remove->setEnabled(true);
			break;
	}
}

/**
 * \brief	an item of the treeview is double clicked, if it is a Keyitem, the grepShortcutKeyDlg is shown
 */
void OptionsTabShortcuts::onItemDoubleClicked(QTreeWidgetItem *item, int column) {
	Q_UNUSED(column);

	/* if we got not a valid or an item with childs, then it's not usable for us, because it is not a key item. */
	if(!item || item->childCount() > 0)
		return;

	/* else show the grep Shortcut Key Dialog */
	grepShortcutKeyDlg *grepIt = new grepShortcutKeyDlg();
	connect(grepIt, SIGNAL(newShortcutKey(QKeySequence)), this, SLOT(onNewShortcutKey(QKeySequence)));
	grepIt->show();
}

/**
 * \brief	slot onNewShortcutKey is called when the grepShortcutKeyDlg has captured a KeySequence,
 *			so the new KeySequence can be set to the KeyItem
 * \param	the new KeySequence for the keyitem
 */
void OptionsTabShortcuts::onNewShortcutKey(QKeySequence key) {
	OptShortcutsUI *d = (OptShortcutsUI *)w;
	QTreeWidgetItem	*keyItem;
	QList<QTreeWidgetItem *> selectedItems = d->treeShortcuts->selectedItems();
	QString	optionsPath;
	Kind itemKind;

	/* we can set the new KeySequence to just one Key item */
	if(selectedItems.count() == 0 || selectedItems.count() > 1)
		return;

	keyItem = selectedItems[0];
	itemKind = (OptionsTabShortcuts::Kind)keyItem->data(0, ITEMKIND).toInt();

	/* if we got a key item, set the new key sequence and notify the options dialog that data has changed */
	if(itemKind == OptionsTabShortcuts::KeyItem) {
		keyItem->setText(1, key.toString());
		emit dataChanged();	
	}
}