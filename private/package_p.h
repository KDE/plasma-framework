#include <plasma.h>
#include <plasma/package.h>
#include <plasma/service.h>

#include <QString>

namespace Plasma
{

class PackagePrivate
{
public:
    PackagePrivate(const PackageStructure::Ptr st, const QString &p);
    ~PackagePrivate();

    /**
     * Publish this package on the network.
     * @param methods the ways to announce this package on the network.
     */
    void publish(AnnouncementMethods methods);

    /**
     * Remove this package from the network.
     */
    void unpublish();

    /**
     * @returns whether or not this service is currently published on the network.
     */
    bool isPublished() const;

    PackageStructure::Ptr structure;
    Service *service;
    bool valid;
};

}

