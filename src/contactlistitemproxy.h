/*
 * contactlistitemproxy.h - proxy item contact list item class
 * Copyright (C) 2008-2010  Yandex LLC (Michail Pishchagin)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#ifndef CONTACTLISTITEMPROXY_H
#define CONTACTLISTITEMPROXY_H

#include <QObject>
#include <QPointer>

class ContactListItem;
class ContactListGroup;

class ContactListItemProxy : public QObject
{
public:
	ContactListItemProxy(ContactListGroup* parent, ContactListItem* item);
	~ContactListItemProxy();

	ContactListItem* item() const { return item_; }
	ContactListGroup* parent() const { return parent_; }

private:
	QPointer<ContactListItem> item_;
	QPointer<ContactListGroup> parent_;
};

#endif
