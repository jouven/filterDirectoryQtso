#ifndef FILTERDIRECTORYQTSO_FILTERDIRECTORY_HPP
#define FILTERDIRECTORYQTSO_FILTERDIRECTORY_HPP

#include "crossPlatformMacros.hpp"

#include "baseClassQtso/baseClassQt.hpp"

#include <QString>
#include <QStringList>
#include <QObject>

#include <string>
#include <vector>

class QMutex;

struct EXPIMP_FILTERDIRECTORYQTSO filterOptions_s
{
    //else use relative (relative to the root directory that's being filtered)
    bool useAbsolutePaths_pub = false;
    //filename wildcard (? and *) filter, Qt entryList(nameFilters...
    //this filter applies "or" wise, if a file matches any it will show in the results
    QStringList filenameWildcardFilters_pub;

    //regex patterns files must match
    //this filter applies "or" wise, if a file matches any it will show in the results
    QStringList filenameRegexFilters_pub;
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
    //it's no the same as navigate directories, this adds directory entries
    //to the results, but directories are navigated depending navigateSubdirectories_pub
    bool listDirectories_pub = false;
    bool listFiles_pub = true;
    bool listHidden_pub = true;

    //TODO datetimes, filesizes
};

//can be reused, not thread-safe, has a threaded filter function to run the filtering on another thread
class EXPIMP_FILTERDIRECTORYQTSO directoryFilter_c : public QObject, public baseClassQt_c
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
    std::vector<state_ec> stateQueue_pri = { state_ec::initial };
    bool pleaseStop_pri = false;

    //for this to be valid check if empty
    QString directoryPath_pri;
    filterOptions_s filterOptions_pri;

    QMutex* stateMutex_pri = nullptr;

    bool isValidDirectoryPath_f();
    bool isValidFilterOptions_f();

    bool filenameRegexMatch_f(const QString& filename_par_con) const;
    bool filenameFullExtensionMatch_f(const QString& filename_par_con) const;
public:
    directoryFilter_c() = delete;
    directoryFilter_c(
            QObject* parent_par,
            const QString& directoryPath_par_con
            , const filterOptions_s& filterOptions_par_con = filterOptions_s()
    );

    //"FUTURE?" when there is a portable way to get a directory size easily
    //add another signal for progress, e.g., "x bytes of y total bytes processed"
    //still this would only work without filters, with filters the total can't be known beforehand

    std::vector<QString> filter_f();
    std::vector<state_ec> stateQueue_f() const;
    state_ec currentState_f() const;

    QString filteredDirectory_f() const;
    filterOptions_s filterOptions_f() const;

    void setDirectoryPath_pri_con(const QString& directoryPath_par_con);
    void setFilterOptions_pri_con(const filterOptions_s& filterOptions_par_con);
Q_SIGNALS:
    //emitted when filter_f validates, calling isValidDirectoryPath_f and isValidFilterOptions_f, at the start and there are errors
    void error_signal();
    //emitted when filter_f finishes in any way
    void finished_signal();
    //emitted at the very end when filterThreaded_f finishes
    void finishedThreaded_signal();
    //emitted when filter_f starts filtering (after validating first)
    void running_signal();
    //emited when filter_f is stopped midway using stopFiltering_f
    void stopped_signal();
    //emitted inside filterThreaded_f when filter_f returns the results to pass them to the function slot/s "waiting" for the results
    void filterThreadedResultsReady_signal(std::vector<QString> result_par);
public Q_SLOTS:
    //connect a slot to filterThreadedResultsReady_signal to grab the results
    void filterThreaded_f();
    void stopFiltering_f();
};

#endif // FILTERDIRECTORYQTSO_FILTERDIRECTORY_HPP
