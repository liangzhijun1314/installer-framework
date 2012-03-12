/**************************************************************************
**
** This file is part of Installer Framework
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef CREATELOCALREPOSITORYOPERATION_H
#define CREATELOCALREPOSITORYOPERATION_H

#include "qinstallerglobal.h"

#include <QtCore/QObject>

namespace QInstaller {

class AutoHelper;

class INSTALLER_EXPORT CreateLocalRepositoryOperation : public QObject, public Operation
{
    Q_OBJECT
    friend class AutoHelper;

public:
    CreateLocalRepositoryOperation();

    void backup();
    bool performOperation();
    bool undoOperation();
    bool testOperation();
    Operation *clone() const;

signals:
    void progressChanged(double progress);
    void outputTextChanged(const QString &message);

private:
    void emitFullProgress();
};

} // namespace QInstaller

#endif // CREATELOCALREPOSITORYOPERATION_H
