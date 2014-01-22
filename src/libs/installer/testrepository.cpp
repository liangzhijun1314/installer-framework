/**************************************************************************
**
** Copyright (C) 2012-2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
**************************************************************************/
#include "testrepository.h"

#include <kdupdaterfiledownloader.h>
#include <kdupdaterfiledownloaderfactory.h>

#include <QtCore/QFile>

using namespace QInstaller;

TestRepository::TestRepository(QObject *parent)
    : KDJob(parent)
    , m_downloader(0)
{
    setTimeout(10000);
    setAutoDelete(false);
    setCapabilities(Cancelable);
}

TestRepository::~TestRepository()
{
    if (m_downloader)
        m_downloader->deleteLater();
}

Repository TestRepository::repository() const
{
    return m_repository;
}

void TestRepository::setRepository(const Repository &repository)
{
    cancel();

    setError(NoError);
    setErrorString(QString());
    m_repository = repository;
}

void TestRepository::doStart()
{
    if (m_downloader)
        m_downloader->deleteLater();

    const QUrl url = m_repository.url();
    if (url.isEmpty()) {
        emitFinishedWithError(InvalidUrl, tr("Empty repository URL."));
        return;
    }

    m_downloader = KDUpdater::FileDownloaderFactory::instance().create(url.scheme(), this);
    if (!m_downloader) {
        emitFinishedWithError(InvalidUrl, tr("URL scheme not supported: %1 (%2).")
            .arg(url.scheme(), url.toString()));
        return;
    }

    QAuthenticator auth;
    auth.setUser(m_repository.username());
    auth.setPassword(m_repository.password());
    m_downloader->setAuthenticator(auth);

    connect(m_downloader, SIGNAL(downloadCompleted()), this, SLOT(downloadCompleted()));
    connect(m_downloader, SIGNAL(downloadAborted(QString)), this, SLOT(downloadAborted(QString)),
        Qt::QueuedConnection);
    connect(m_downloader, SIGNAL(authenticatorChanged(QAuthenticator)), this,
        SLOT(onAuthenticatorChanged(QAuthenticator)));

    m_downloader->setAutoRemoveDownloadedFile(true);
    m_downloader->setUrl(QUrl(url.toString() + QString::fromLatin1("/Updates.xml")));

    m_downloader->download();
}

void TestRepository::doCancel()
{
    if (m_downloader) {
        QString errorString = m_downloader->errorString();
        if (errorString.isEmpty())
            errorString = tr("Got a timeout while testing: '%1'").arg(m_repository.displayname());
        // at the moment the download sends downloadCompleted() if we cancel it, so just
        disconnect(m_downloader, 0, this, 0);
        m_downloader->cancelDownload();
        emitFinishedWithError(KDJob::Canceled, errorString);
    }
}

void TestRepository::downloadCompleted()
{
    QString errorMsg;
    int error = DownloadError;

    if (m_downloader->isDownloaded()) {
        QFile file(m_downloader->downloadedFileName());
        if (file.exists() && file.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            QString errorMsg;
            if (!doc.setContent(&file, &errorMsg)) {
                error = InvalidUpdatesXml;
                errorMsg = tr("Could not parse Updates.xml! Error: %1.").arg(errorMsg);
            } else {
                error = NoError;
            }
        } else {
            errorMsg = tr("Updates.xml could not be opened for reading!");
        }
    } else {
        errorMsg = tr("Updates.xml could not be found on server!");
    }

    if (error > NoError)
        emitFinishedWithError(error, errorMsg);
    else
        emitFinished();

    m_downloader->deleteLater();
    m_downloader = 0;
}

void TestRepository::downloadAborted(const QString &reason)
{
    emitFinishedWithError(DownloadError, reason);
}

void TestRepository::onAuthenticatorChanged(const QAuthenticator &authenticator)
{
    m_repository.setUsername(authenticator.user());
    m_repository.setPassword(authenticator.password());
}