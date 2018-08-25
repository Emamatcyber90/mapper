/*
 *    Copyright 2018 Kai Pastor
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


#include "icon_properties_widget.h"

#include <QSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>

#include "core/symbols/symbol.h"
#include "gui/file_dialog.h"
#include "gui/util_gui.h"
#include "gui/symbols/symbol_setting_dialog.h"
#include "util/backports.h"

namespace OpenOrienteering {

namespace {

QString pngFileFilter()
{
	static const QString filter_template(QLatin1String("%1 (%2)"));
	QStringList filters = {
	    filter_template.arg(::OpenOrienteering::IconPropertiesWidget::tr("PNG"), QLatin1String("*.png")),
	    ::OpenOrienteering::IconPropertiesWidget::tr("All files (*.*)")
	};
	return filters.join(QLatin1String(";;"));
}


}  // namespace



IconPropertiesWidget::IconPropertiesWidget(Symbol* symbol, SymbolSettingDialog* dialog)
: QWidget(dialog)
, dialog(dialog)
, symbol(symbol)
{
	auto layout = new QFormLayout();
	setLayout(layout);
	
	layout->addRow(Util::Headline::create(tr("Default icon")));
	
	default_icon_display = new QLabel();
	layout->addRow(default_icon_display);
	
	default_icon_size_edit = Util::SpinBox::create(22, 256, tr("px"));
	layout->addRow(tr("Width:"), default_icon_size_edit);
	
	layout->addItem(Util::SpacerItem::create(this));
	
	
	layout->addRow(Util::Headline::create(tr("Custom icon")));
	
	custom_icon_display = new QLabel();
	layout->addRow(custom_icon_display);
	
	custom_icon_size_edit = new QLineEdit();
	custom_icon_size_edit->setDisabled(true);
	layout->addRow(tr("Width:"), custom_icon_size_edit);
	
	layout->addItem(Util::SpacerItem::create(this));
	
	
	auto buttons_layer = new QHBoxLayout();
	export_button = new QPushButton(tr("Export..."));
	buttons_layer->addWidget(export_button);
	import_button = new QPushButton(tr("Import..."));
	buttons_layer->addWidget(import_button);
	clear_button = new QPushButton(tr("Clear"));
	buttons_layer->addWidget(clear_button);
	buttons_layer->addStretch(1);
	
	layout->addRow(buttons_layer);
	
	
	connect(default_icon_size_edit, QOverload<int>::of(&QSpinBox::valueChanged), this, &IconPropertiesWidget::sizeEdited, Qt::QueuedConnection);
	connect(export_button, &QAbstractButton::clicked, this, &IconPropertiesWidget::exportClicked);
	connect(import_button, &QAbstractButton::clicked, this, &IconPropertiesWidget::importClicked);
	connect(clear_button, &QAbstractButton::clicked, this, &IconPropertiesWidget::clearClicked);
	
	updateWidgets();
}


IconPropertiesWidget::~IconPropertiesWidget() = default;


void IconPropertiesWidget::reset(Symbol* symbol)
{
	Q_ASSERT(symbol);
	this->symbol = symbol;
	updateWidgets();
}


void IconPropertiesWidget::updateWidgets()
{
	auto custom_icon = symbol->getCustomIcon();
	symbol->setCustomIcon({});
	auto default_icon = symbol->getIcon(dialog->getSourceMap());
	
	default_icon_display->setPixmap(QPixmap::fromImage(default_icon));
	default_icon_size_edit->setValue(default_icon.width());
	
	custom_icon_display->setPixmap(QPixmap::fromImage(custom_icon));
	if (custom_icon.isNull())
	{
		custom_icon_size_edit->setText(QLatin1String("-"));
		clear_button->setEnabled(false);
	}
	else
	{
		custom_icon_size_edit->setText(tr("%1 px").arg(custom_icon.width()));
		clear_button->setEnabled(true);
	}
	
	symbol->setCustomIcon(custom_icon);
}


void IconPropertiesWidget::sizeEdited(int size)
{
	if (default_icon_display->pixmap()->width() != size
	    && dialog->getSourceMap())
	{
		auto icon = symbol->createIcon(*dialog->getSourceMap(), size);
		default_icon_display->setPixmap(QPixmap::fromImage(icon));
	}
}


void IconPropertiesWidget::exportClicked()
{
	auto path = FileDialog::getSaveFileName(this, tr("Export symbol icon ..."), {}, pngFileFilter());
	if (path.isEmpty())
		return;
	
	if (!path.endsWith(QLatin1String(".png", Qt::CaseInsensitive)))
		path.append(QLatin1String(".png"));
	
	auto icon = symbol->getCustomIcon();
	if (icon.isNull())
		icon = default_icon_display->pixmap()->toImage();
	
	QImageWriter writer{path};
	if (!writer.write(icon))
	{
		QMessageBox::warning(this, tr("Error"),
		                     tr("Failed to save the image:\n%1")
		                     .arg(writer.errorString()) );
	}
}


void IconPropertiesWidget::importClicked()
{
	auto path = FileDialog::getOpenFileName(this, tr("Import symbol icon ..."), {}, pngFileFilter());
	if (path.isEmpty())
		return;
	
	QImageReader reader{path};
	auto icon = reader.read();
	if (icon.isNull())
	{
		QMessageBox::warning(this, tr("Error"),
		                     tr("Cannot open file:\n%1\n\n%2").
		                     arg(path, reader.errorString()) );
		return;
	}
	
	symbol->setCustomIcon(icon);
	updateWidgets();
	emit iconModified();
}


void IconPropertiesWidget::clearClicked()
{
	symbol->setCustomIcon({});
	updateWidgets();
	emit iconModified();
}



}  // namespace OpenOrienteering
