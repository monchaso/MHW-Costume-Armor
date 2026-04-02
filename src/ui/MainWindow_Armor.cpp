#include "Config.h"
#include "MainWindow.hpp"
#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMovie>
#include <QStandardItem>
#include <QThread>

/// These file contains Member definitions of the MainWindow class
/// Related to The managment of input Comboboxes

void MainWindow::_translateArmorData() {
  QAction *checked = _langGroup->checkedAction();
  _settings.setValue("General/Language", checked->text());
  auto Lang = checked->text();

  QFont font;
  for (auto lAction : _langGroup->actions())
    lAction->setFont(font);

  font.setBold(true);
  checked->setFont(font);

  for (int i = 0; i < 5; ++i)
    _sourceModels[i]->clear();

  this->_settings.sync();

  bool lr = _settings.value("General/LRArmors", false).toBool();

  // translate comboboxes
  auto &tArmorData = armorData.at(_langGroup->checkedAction()->text());
  for (uint id = 0; id < armor_count(); id++) {
    if (id < 37 && !lr)
      continue;
    uint mode = armorMode.at(id);
    auto &name = tArmorData.at(id);
    for (int i = 0; i < 5; i++) {
      if (mode & (1 << i)) {
        QStandardItem *item = new QStandardItem(name);
        item->setData(id, Qt::UserRole);
        _sourceModels[i]->appendRow(item);
      }
    }
  }

  for (int i = 0; i < 5; i++) {
    _sourceModels[i]->sort(0);
    QStandardItem *nothing = new QStandardItem("Nothing");
    nothing->setData(Armor::NOTHING, Qt::UserRole);
    _sourceModels[i]->insertRow(0, nothing);
    this->_inputBoxes[i]->setCurrentIndex(0);
    _selectedArmorIds[i] = Armor::NOTHING;
    // After re-populating, restore the previously selected value if possible
    int idx = _inputBoxes[i]->findData(_selectedArmorIds[i]);
    if (idx >= 0) {
      _inputBoxes[i]->setCurrentIndex(idx);
    } else {
      _inputBoxes[i]->setCurrentIndex(0); // Fallback to "Nothing"
      _selectedArmorIds[i] = Armor::NOTHING;
    }
  }
}

void MainWindow::_updateArmorValues() {
  auto Data = _MHManager.getPlayerData();
  int index;
  for (int i = 0; i < 5; ++i) {
    if (_proxyModels[i]) {
      _proxyModels[i]->setSelectedId(Data[i]);
    }
    _selectedArmorIds[i] =
        Data[i]; // Synchronize _selectedArmorIds with fetched data
    index = _inputBoxes[i]->findData(_selectedArmorIds[i]);
    if (index < 0) {
      index = 0;
      _selectedArmorIds[i] =
          Armor::NOTHING; // Update _selectedArmorIds if value is unknown
      LOG_ENTRY(WARNING, "Encountered unknown value ("
                             << Data[i] << ") changing it to " << Armor::NOTHING
                             << " for safety");
      if (_proxyModels[i]) {
        _proxyModels[i]->setSelectedId(Armor::NOTHING);
      }
    } else {
      _selectedArmorIds[i] = Data[i];
    }
    _inputBoxes[i]->setCurrentIndex(index);
  }
}

void MainWindow::_clearArmor() {
  for (int i = 0; i < 5; ++i)
    _MHManager.getPlayerData()[i] = Armor::NOTHING;
  this->_updateArmorValues();
}

bool MainWindow::_parseInputBoxes() {
  int Val;
  for (int i = 0; i < 5; ++i) {
    Val = _selectedArmorIds[i];
    if (Val == 0) // Means Error btw
    {
      DialogWindow *Dia = new DialogWindow(
          this, "ERROR", "Invalid Value for armor", Status::ERROR0);
      Dia->show();
      this->_fetchData(true);
      return false;
    }
    this->_MHManager.getPlayerData()[i] = Val;
  }
  return true;
}

void MainWindow::_changeAll() {
  QStringList names;
  QList<uint> idx;
  uint i;
  bool lr = _settings.value("General/LRArmors", false).toBool();
  QString filter = _searchBar->text();
  auto &tArmorData = armorData.at(_langGroup->checkedAction()->text());
  for (i = 0; i < armor_count(); i++) {
    if (armorMode.at(i) == 31 && (i >= 37 || lr)) // Means its a set
    {
      auto &name = tArmorData.at(i);
      if (filter.isEmpty() || name.contains(filter, Qt::CaseInsensitive)) {
        names << name;
        idx << i;
      }
    }
  }

  auto [aName, aId, ok] =
      getItemInputDialog(this, "Change All Armor", "Select set: ", names, idx);
  Q_UNUSED(aName);
  if (!ok)
    return;
  for (i = 0; i < 5; ++i)
    _MHManager.getPlayerData()[i] = aId;

  this->_updateArmorValues();
}

void MainWindow::_manualInputValue() {
  bool ok;
  int id = QInputDialog::getInt(this, "Manually Input ID", "Input Armor ID", 1,
                                1, 2147483647, 1, &ok);
  if (!ok)
    return;
  static auto customValue = QStringLiteral("Custom ID : %1");
  auto cid = customValue.arg(id);
  for (int i = 0; i < 5; ++i) {
    QStandardItem *item = new QStandardItem(cid);
    item->setData(id, Qt::UserRole);
    _sourceModels[i]->insertRow(0, item);
    this->_inputBoxes[i]->setCurrentIndex(0);
    _selectedArmorIds[i] = id;
  }
}

void MainWindow::_toggleLRArmors() {
  _settings.setValue("General/LRArmors",
                     !_settings.value("General/LRArmors", false).toBool());
  _settings.sync();
  _translateArmorData();
}
void MainWindow::_filterArmor(const QString &text) {
  Q_UNUSED(text);
  _applyArmorFilter();
}

void MainWindow::_applyArmorFilter() {
  QString filter = _searchBar->text();
  for (int i = 0; i < 5; ++i) {
    if (_proxyModels[i]) {
      _proxyModels[i]->setFilterFixedString(filter);
    }
  }
}

void MainWindow::_onArmorSelectionChanged() {
  for (int i = 0; i < 5; ++i) {
    uint val = _inputBoxes[i]->currentData().toUInt();
    // Only update if it's a valid armor ID (not partially matching or error)
    if (val != 0) {
      _selectedArmorIds[i] = val;
      if (_proxyModels[i]) {
        _proxyModels[i]->setSelectedId(val);
      }
    }
  }
}
