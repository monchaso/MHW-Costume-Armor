#pragma once
#include "AboutWindow.hpp"
#include "ArmorData.hpp"
#include "DialogWindow.hpp"
#include "MHMemory.hpp"
#include "UpdaterGithub.hpp"
#include "ui_MainWindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMainWindow>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <array>
#include <iomanip>
#include <set>

class ArmorFilterProxyModel : public QSortFilterProxyModel {
public:
  explicit ArmorFilterProxyModel(QObject *parent = nullptr)
      : QSortFilterProxyModel(parent), m_selectedId(0) {}

  void setSelectedId(uint id) {
    if (m_selectedId != id) {
      m_selectedId = id;
      invalidateFilter();
    }
  }

protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex &source_parent) const override {
    QModelIndex index =
        sourceModel()->index(source_row, filterKeyColumn(), source_parent);
    uint id = sourceModel()->data(index, Qt::UserRole).toUInt();
    if (id == m_selectedId && id != 0) {
      return true; // Always show the selected item
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
  }

private:
  uint m_selectedId;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QString github_repo, QString current_version, QSettings &settings,
             QWidget *parent = 0);
  ~MainWindow();
  QFile savedSetsFile =
      QCoreApplication::applicationDirPath() + "\\SavedSets.json";
  inline static constexpr float sizeRatio{1.6};

private:
  Ui::MainWindow *ui;
  QActionGroup *_versionGroup = nullptr;
  QActionGroup *_langGroup = nullptr;
  QActionGroup *_logGroup = nullptr;
  MH_Memory _MHManager;
  QSettings &_settings;
  QJsonDocument _savedSetsDocument;
  QVariantMap _savedSets;
  std::array<QComboBox *, 5> _inputBoxes{nullptr};
  UpdaterGithub _updater;
  QMap<QWidget *, QPair<QSize, QPoint>> _baseGeo;
  void _setFontSize();
  QWidgetList _childCache;
  uint _qssLen;
  QLineEdit *_searchBar = nullptr;
  std::array<ArmorFilterProxyModel *, 5> _proxyModels{nullptr};
  std::array<QStandardItemModel *, 5> _sourceModels{nullptr};
  std::array<uint, 5> _selectedArmorIds{0};

protected:
  bool _ensureSizeRatio(QResizeEvent *event);
  void _resizeWidgets();
  void resizeEvent(QResizeEvent *event) final;

private slots:
  // Inits
  void _loadJsonFiles();
  void _initVersionSelector();
  void _initLanguages();
  void _initSearchBar();
  // Dialogs
  void _checkForUpdates();
  void _aboutInfo();
  void _notImplemented();
  // Memory
  void _setupForSearch(bool silent);
  void _findAddress();
  void _writeData();
  void _fetchData(bool noMessage = false);
  // Settings
  void _updateSelectedLogLevel();
  void _updateSelectedVersion();
  void _getCustomSteamPath();
  void _toggleNoBackup();
  void _setAutoSteam();
  void _toggleAutoUpdates();
  void _fontScale();
  void _toggleLRArmors();
  // Saved Sets
  bool _flushSavedSets();
  void _saveCurrentSet();
  void _deleteCurrentSet();
  void _loadSavedSet();
  void _loadSavedSetPopup();
  // Armor
  void _translateArmorData();
  bool _parseInputBoxes();
  void _updateArmorValues();
  void _clearArmor();
  void _changeAll();
  void _manualInputValue();
  void _filterArmor(const QString &text);
  void _onArmorSelectionChanged();
  void _applyArmorFilter();
};
