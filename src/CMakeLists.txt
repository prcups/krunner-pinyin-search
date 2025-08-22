add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.krunner_pinyin_search\")

kcoreaddons_add_plugin(krunner_pinyin_search INSTALL_NAMESPACE "kf6/krunner")

target_sources(krunner_pinyin_search PRIVATE
    pinyinsearch.cpp
    pinyinmatch.cpp
    res.qrc
)

target_link_libraries(krunner_pinyin_search
    Qt6::Gui # QAction
    KF6::Runner
    KF6::I18n
    KF6::Service
    KF6::KIOCore
    KF6::KIOGui
    KF6::CoreAddons
    KF6::JobWidgets
)
