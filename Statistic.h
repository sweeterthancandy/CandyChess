#ifndef STATISTIC_H
#define STATISTIC_H
        
namespace CC{
        /*
           Part of the game should also be statistics. These will be things
           like the point difference in the game, which will be used in
           the game tree construction to give the builder the context
           to figure out where to short circute the game tree.
                A statistic is either a series statistic, which gets
           updated every move, or a static statistic, which gets 
           computed fresh every move. I guess everything could be both, but
           we want to keep the state of statistics such as point, rather
           than pointless computation
         */
        struct Statistic;
        struct StatisticManager;

        struct Statistic{
                enum StatisticsKind{
                        SK_Single,
                        SK_Delta,
                };

                // I think this should be fine grained, virtual destructores
                // remove alot of optmizations
                virtual ~Statistic()=0;
                virtual std::shared_ptr<Statistic> Clone()=0;
        };
        
        struct DeltaStatistic : Statistic{
                virtual void Delta(StatisticManager* mgr, ChessInstance* instance)=0;
        };

        struct StatisticFactory{
                virtual std::shared_ptr<Statistic> Create(StatisticManager* mgr, ChessInstance* instance)=0;
        };

        struct StatisticRegistrar{
        };
        /*
           We will have statistics that depend on other statistics, so we
           have a manager which will return a common instance
        */
        struct StatisticManager{
        };

        namespace Stats{

                struct Points : Statistic{
                };
                struct TurnNumber : Statistics{
                };
                struct CheckOrCheckMate : Statistic{
                };

        } // end namespace Stats
} // end namespace CC

#endif // STATISTIC_H
