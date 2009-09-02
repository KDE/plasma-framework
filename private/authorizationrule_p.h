#include <plasma/authorizationrule.h>

namespace Plasma
{

class AuthorizationRulePrivate {
public:
    AuthorizationRulePrivate(const QString &serviceName, const QString &credentialID);
    ~AuthorizationRulePrivate();
    bool matches(const QString &serviceName, const QString &credentialID) const;

    QString serviceName;
    QString credentialID;
    bool PINvalidated;
    QString pin;
    AuthorizationRule::Policy policy;
    AuthorizationRule::Targets targets;
    AuthorizationRule::Persistence persistence;
};

}
