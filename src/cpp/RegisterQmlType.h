#pragma once

#include "PageListView/ZPageListView.h"
#include "Wizard/ZWizardFramework.h"
#include "Http/ZHttp.h"

static void registerQmlType() {
    qmlRegisterType<ZPageListView>("ZozQuick", 1, 0, "ZPageListView");
    qmlRegisterType<ZDataState>("ZozQuick", 1, 0, "ZDataState");

    qmlRegisterType<HoleArea>("ZozQuick", 1, 0, "ZHoleArea");
    qmlRegisterType<RoundRectangle>("ZozQuick", 1, 0, "ZRoundRectangle");
    qmlRegisterType<RoundPolygon>("ZozQuick", 1, 0, "ZRoundPolygon");
    qmlRegisterSingletonType<WizardFramework>("ZozQuick", 1, 0, "ZWizard", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        WizardFramework *obj = new WizardFramework();
        return obj;
    });

    registerHttpToQml("ZozQuick");

    // 注册命名空间
//    qmlRegisterUncreatableMetaObject(
//                Zoz::staticMetaObject,
//                "ZozQuick",
//                1, 0,
//                "Zoz",
//                "Error: only enums"
//                );
}

