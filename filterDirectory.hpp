#ifndef FILTERDIRECTORYQTSO_FILTERDIRECTORY_HPP
#define FILTERDIRECTORYQTSO_FILTERDIRECTORY_HPP

#include "crossPlatformMacros.hpp"

#include "baseClassQtso/baseClassQt.hpp"

#include <QString>
#include <QStringList>
#include <QObject>

#include <string>
#include <vector>

struct EXPIMP_FILTERDIRECTORYQTSO filterOptions_s
{
    //else use relative (relative to the root directory that's being filtered)
    bool useAbsolutePaths_pub = false;
    //generic filename filter, Qt entryList(nameFilters...
    //this filter applies "or" wise, if a file matches any it will show in the results
    QStringList filenameFilters_pub;
    //extensions, the whole extension has to match, see Qt QFileInfo::completeSuffix
    //this filter applies "or" wise, if a file matches any it will show in the results
    QStringList filenameFullExtensions_pub;
    //when searching
    bool navigateSubdirectories_pub = true;
    //when searching
    bool navigateHiddenDirectories_pub = true;
    //when searching, only list files from directories with a certain named file
    QString onlyIncludeDirectoriesWithFileX_pub;

    bool listEmptyDirectories_pub = true;
    //it's no the same as navigate directories, this one just removes directory name entries
    //from the results, but directories are navigated
    bool listDirectories_pub = false;
    bool listFiles_pub = true;
    bool listHidden_pub = true;

    //TODO datetimes, filesizes, contains
};

//can be reused, not thread-safe
class EXPIMP_FILTERDIRECTORYQTSO directoryFilter_c : public QObject, public eines::baseClassQt_c
{
    Q_OBJECT
public:
    enum class state_ec
    {
        initial,
        running,
        stopped,
        error,
        finished
    };
private:
    std::vector<state_ec> stateQueue_pri;
    bool pleaseStop_pri = false;

    //for this to be valid check if empty
    QString directoryPath_pri;
    filterOptions_s filterOptions_pri;

    bool isValidDirectoryPath_f();
    bool isValidFilterOptions_f();
public:
    directoryFilter_c() = default;
    directoryFilter_c(
            const QString directoryPath_par_con
            , const filterOptions_s filterOptions_par_con = filterOptions_s()
    );

    std::vector<QString> filter_f();
    std::vector<state_ec> stateQueue_f() const;
    state_ec currentState_f() const;
    void stopFiltering_f();

    QString filteredDirectory_f() const;
    filterOptions_s filterOptions_f() const;

    void setDirectoryPath_pri_con(const QString& directoryPath_par_con);
    void setFilterOptions_pri_con(const filterOptions_s& filterOptions_par_con);
Q_SIGNALS:
    void error_signal();
    void finished_signal();
    void running_signal();
    void stopped_signal();
};

#endif // FILTERDIRECTORYQTSO_FILTERDIRECTORY_HPP
