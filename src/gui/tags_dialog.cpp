/*
 *    Copyright 2012, 2013 Jan Dalheimer
 *
 *    This file is part of OpenOrienteering.
 *
 *    OpenOrienteering is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    OpenOrienteering is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with OpenOrienteering.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tags_dialog.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <QDebug>

#include "../object.h"

//TODO: Tool to open this dialog (instead of normal edit tool + key)

TagsDialog::TagsDialog(Object *object, QWidget *parent)
 : QDialog(parent),
   object(object)
{
	tags_table = new QTableWidget(this);
	tags_table->setSelectionMode(QTableWidget::SingleSelection);
	add_tag_button = new QPushButton(tr("Add"), this);
	remove_tag_button = new QPushButton(tr("Remove"), this);
	close_button = new QPushButton(tr("Close"), this);

	buttons_layout = new QHBoxLayout;
	layout = new QVBoxLayout;

	buttons_layout->addWidget(add_tag_button);
	buttons_layout->addWidget(remove_tag_button);
	buttons_layout->addWidget(close_button);
	layout->addWidget(tags_table);
	layout->addLayout(buttons_layout);
	this->setLayout(layout);

	this->setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint & ~Qt::WindowSystemMenuHint);

	const Object::Tags& tags = object->tags();
	int rows = tags.size();
	tags_table->setColumnCount(2);
	tags_table->setRowCount(rows);
	tags_table->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));
	int row = 0;
	for (Object::Tags::const_iterator tag = tags.constBegin(), end = tags.constEnd(); tag != end; ++tag, ++row)
	{
		tags_table->setItem(row, 0, new QTableWidgetItem(tag.key()));
		tags_table->setItem(row, 1, new QTableWidgetItem(tag.value()));
	}

	connect(add_tag_button, SIGNAL(clicked()), this, SLOT(addTagClicked()));
	connect(remove_tag_button, SIGNAL(clicked()), this, SLOT(removeTagClicked()));
	connect(close_button, SIGNAL(clicked()), this, SLOT(closeClicked()));
}

void TagsDialog::addTagClicked()
{
	QString key(tr("key"));
	for (int i = 2; keyExists(key); ++i)
	{
		key = tr("key %0").arg(i);
	}

	const int row = tags_table->rowCount();
	tags_table->setRowCount(row + 1);
	tags_table->setItem(row, 0, new QTableWidgetItem(key));
	tags_table->setItem(row, 1, new QTableWidgetItem(""));
}

void TagsDialog::removeTagClicked()
{
	if (tags_table->selectedItems().isEmpty())
		return;

	tags_table->removeRow(tags_table->selectedItems()[0]->row());
}

void TagsDialog::closeClicked()
{
	Object::Tags tags;
	for (int i = 0; i < tags_table->rowCount(); i++)
	{
		QString key = tags_table->item(i, 0)->text();
		QString value = tags_table->item(i, 1)->text();
		tags.insert(key, value);
	}
	object->setTags(tags);
	
	this->accept();
}

bool TagsDialog::keyExists(const QString &key) const
{
	for (int i = 0; i < tags_table->rowCount(); i++)
	{
		if (tags_table->item(i, 0)->text() == key)
			return true;
	}
	return false;
}