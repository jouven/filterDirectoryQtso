#include "filterDirectory.hpp"

#include <QDir>
#include <QFileInfo>


void directoryFilter_c::setDirectoryPath_pri_con(const QString& directoryPath_par_con)
{
    directoryPath_pri = directoryPath_par_con;
}

void directoryFilter_c::setFilterOptions_pri_con(const filterOptions_s& filterOptions_par_con)
{
    filterOptions_pri = filterOptions_par_con;
}

filterOptions_s directoryFilter_c::filterOptions_f() const
{
    return filterOptions_pri;
}

bool directoryFilter_c::isValidDirectoryPath_f()
{
    bool resultTmp(false);
    while (true)
    {
        if (directoryPath_pri.isEmpty())
        {
            appendError_f("Directory path is empty");
            break;
        }

        QFileInfo directoryPathFileInfoTmp(directoryPath_pri);
        if (directoryPathFileInfoTmp.exists())
        {
            //good
        }
        else
        {
            appendError_f("Directory path doesn't exist, path: " + directoryPath_pri);
            break;
        }

        if (directoryPathFileInfoTmp.isDir())
        {
            //good
        }
        else
        {
            appendError_f("Directory path is not a directory, path: " + directoryPath_pri);
            break;
        }
        resultTmp = true;
    }
    if (anyError_f())
    {
        stateQueue_pri.emplace_back(state_ec::error);
        Q_EMIT error_signal();
    }
    return resultTmp;
}

bool directoryFilter_c::isValidFilterOptions_f()
{
    bool resultTmp(false);
    while (true)
    {
        if (filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub.isEmpty())
        {
            //no problem here
        }
        else
        {
            if (filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub.size() > 255)
            {
                appendError_f(
                            "Include directories with file X filename length too long, length: "
                            + QString::number(filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub.size())
                            + ", max supported length is 255"
                );
                break;
            }
            else
            {
                //0-255
            }
        }

        if (filterOptions_pri.filenameFilters_pub.isEmpty())
        {
            //no problem here
        }
        else
        {
            for (const QString& filenameFilter_ite_con : filterOptions_pri.filenameFilters_pub)
            {
                if (filenameFilter_ite_con.size() > 255)
                {
                    appendError_f("Filename filter length too long, length: " + QString::number(filenameFilter_ite_con.size()) + ", max supported length is 255");
                    break;
                }
                else
                {
                    //0-255
                }
            }
        }

        if (filterOptions_pri.filenameFullExtensions_pub.isEmpty())
        {
            //no problem here
        }
        else
        {
            for (const QString& filenameFullExtension_ite_con : filterOptions_pri.filenameFullExtensions_pub)
            {
                if (filenameFullExtension_ite_con.size() > 254)
                {
                    appendError_f("Filename full extension filter length too long, length: " + QString::number(filenameFullExtension_ite_con.size()) + ", max supported length is 254");
                    break;
                }
                else
                {
                    //0-254
                }
            }
        }
        resultTmp =true;
        break;
    }
    if (anyError_f())
    {
        stateQueue_pri.emplace_back(state_ec::error);
        Q_EMIT error_signal();
    }
    return resultTmp;
}

directoryFilter_c::directoryFilter_c(
        const QString directoryPath_par_con
        , const filterOptions_s filterOptions_par_con)
    :
      directoryPath_pri(directoryPath_par_con)
    , filterOptions_pri(filterOptions_par_con)
{}

std::vector<QString> directoryFilter_c::filter_f()
{
    std::vector<QString> resultTmp;
    while (true)
    {
        stateQueue_pri = { state_ec::initial };
        if (isValidDirectoryPath_f() and isValidFilterOptions_f())
        {
            //good
        }
        else
        {
            break;
        }
        stateQueue_pri.emplace_back(state_ec::running);
        Q_EMIT running_signal();
        QDir sourceDir(directoryPath_pri);

        QString basePathTmp;
        if (filterOptions_pri.useAbsolutePaths_pub)
        {
            basePathTmp = sourceDir.absolutePath();
        }
        else
        {
            basePathTmp = ".";
        }

        QDir::Filters navigateFiltersTmp(QDir::NoDotAndDotDot);
        if (filterOptions_pri.navigateSubdirectories_pub)
        {
            navigateFiltersTmp = navigateFiltersTmp | QDir::AllDirs;
        }
        if (filterOptions_pri.navigateHiddenDirectories_pub)
        {
            navigateFiltersTmp = navigateFiltersTmp | QDir::Hidden;
        }

        QDir::Filters listFiltersTmp(QDir::NoDotAndDotDot);
        if (filterOptions_pri.listDirectories_pub)
        {
            listFiltersTmp = listFiltersTmp | QDir::AllDirs;
        }
        if (filterOptions_pri.listFiles_pub)
        {
            listFiltersTmp = listFiltersTmp | QDir::Files;
        }
        if (filterOptions_pri.listHidden_pub)
        {
            listFiltersTmp = listFiltersTmp | QDir::Hidden;
        }
        //if the setting is empty
        //consider it found
        bool xFileFoundRoot(filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub.isEmpty());
        if (not xFileFoundRoot)
        {
            //source dir, get the root files
            QStringList rootFileListUnfiltered(sourceDir.entryList(QDir::Files | (filterOptions_pri.navigateHiddenDirectories_pub ? QDir::Hidden : QDir::Files) | QDir::NoDotAndDotDot));
            for (const QString& filename_ite_con : rootFileListUnfiltered)
            {
                if (filename_ite_con == filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub)
                {
                    xFileFoundRoot = true;
                    break;
                }
            }
        }


        if (xFileFoundRoot)
        {
            //source dir, get the root files
            QStringList rootFileList(sourceDir.entryList(filterOptions_pri.filenameFilters_pub, listFiltersTmp));
            for (const QString& filename_ite_con : rootFileList)
            {
                //QOUT_TS("filename_ite_con " << filename_ite_con << endl);
                resultTmp.emplace_back(basePathTmp + "/" + filename_ite_con);
                if (pleaseStop_pri)
                {
                    break;
                }
            }
        }

        if (filterOptions_pri.navigateSubdirectories_pub and not pleaseStop_pri)
        {
            //source dir, get the all the subfolders (names) of the base folder
            //there is no "0" value filter, reuse "dirs" for the inline if
            QStringList subfolders(sourceDir.entryList(QDir::Dirs | (filterOptions_pri.navigateHiddenDirectories_pub ? QDir::Hidden : QDir::Dirs) | QDir::NoDotAndDotDot | QDir::NoSymLinks));

            //while subfolders keep being found
            while (not subfolders.isEmpty() and not pleaseStop_pri)
            {
                QStringList newSubfoldersTmp;
                //for every subfolder gathered
                for (const auto& subfolder_ite_con : subfolders)
                {
                    //qDebug() << "folder_ite_con " << subfolder_ite_con << endl;

                    //do a QDir of the subfolder, using the initial folder + all the subfolder "depth" that has been traveled/iterated
                    QDir currentSubfolderDirTmp(sourceDir.absolutePath() + "/" + subfolder_ite_con);

                    //get the subfolders of the one it's iterating
                    QStringList subFoldersTmp(currentSubfolderDirTmp.entryList(QDir::Dirs | (filterOptions_pri.listHidden_pub ? QDir::Hidden : QDir::Dirs) | QDir::NoDotAndDotDot | QDir::NoSymLinks));
                    //for the found subfolder, prepend the previous subfolder path string
                    for (auto& subfolderTmp_ite : subFoldersTmp)
                    {
                        //qDebug() << "subfolder_ite_con + "/" + subfolderTmp_ite " << subfolder_ite_con + "/" + subfolderTmp_ite << endl;
                        QDir subfolderDirTmp(subfolderTmp_ite);
                        if (subfolderDirTmp.isEmpty())
                        {
                            continue;
                        }
                        //prepend the parent subfolder
                        subfolderTmp_ite.prepend(subfolder_ite_con + "/");
                    }
                    newSubfoldersTmp.append(subFoldersTmp);

                    //if the setting is empty
                    //account it as it is found
                    bool xFileFoundSubdirectory(filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub.isEmpty());
                    if (not xFileFoundSubdirectory)
                    {
                        //get the files of the subfolder
                        QStringList subDirectoryFileListUnfiltered(currentSubfolderDirTmp.entryList(QDir::Files | (filterOptions_pri.navigateHiddenDirectories_pub ? QDir::Hidden : QDir::Files) | QDir::NoDotAndDotDot));
                        for (const QString& filename_ite_con : subDirectoryFileListUnfiltered)
                        {
                            if (filename_ite_con == filterOptions_pri.onlyIncludeDirectoriesWithFileX_pub)
                            {
                                xFileFoundSubdirectory = true;
                                break;
                            }
                        }
                    }

                    if (xFileFoundSubdirectory)
                    {
                        //get the files of the subfolder
                        QStringList subDirectoryFileList(currentSubfolderDirTmp.entryList(filterOptions_pri.filenameFilters_pub, listFiltersTmp));
                        for (const auto& filename_ite_con : subDirectoryFileList)
                        {
                            //qDebug() << "source_par_con.absoluteFilePath() + "/" + subfolder_ite_con + "/" + filename_ite_con " << source_par_con.absoluteFilePath() + "/" + subfolder_ite_con + "/" + filename_ite_con << endl;
                            resultTmp.emplace_back(basePathTmp + "/" + subfolder_ite_con + "/" + filename_ite_con);
                        }
                    }
                    if (pleaseStop_pri)
                    {
                        break;
                    }
                }

                //assign the new subfolders to the previous subfolders
                //qDebug() << "newFoundFolders.size() " << newFoundFolders.size() << endl;
                subfolders = newSubfoldersTmp;
            }
        }
        if (pleaseStop_pri)
        {
            pleaseStop_pri = false;
            stateQueue_pri.emplace_back(state_ec::stopped);
            Q_EMIT stopped_signal();
        }
        else
        {
            stateQueue_pri.emplace_back(state_ec::finished);
            Q_EMIT finished_signal();
        }
        break;
    }
    return resultTmp;
}

std::vector<directoryFilter_c::state_ec> directoryFilter_c::stateQueue_f() const
{
    return stateQueue_pri;
}

directoryFilter_c::state_ec directoryFilter_c::currentState_f() const
{
    return stateQueue_pri.back();
}

QString directoryFilter_c::filteredDirectory_f() const
{
    return directoryPath_pri;
}

void directoryFilter_c::stopFiltering_f()
{
    if (not pleaseStop_pri)
    {
        pleaseStop_pri = true;
    }
}

