#ifndef TIMESERVICE_H
#define TIMESERVICE_H

class TimeService
{
public:
    TimeService(int port);
    bool isSynchronized();

private:
    bool synchronized;

    std::pair<time_t, time_t> synchronize(time_t deliveryTime);
};

#endif // TIMESERVICE_H
