#include "IndependentParametersModel.h"
#include "ProgramSignature.h"
#include "TuningDatabase.h"
#include "svm.h"

#include <numeric>

using std::make_pair;
using std::map;
using std::vector;
using std::string;


namespace {
/*
 * Helper function for SVM learning
 */

std::vector<string>getFeatureNames( ProgramSignature current ) {
    vector<ProgramSignature> signatures;
    signatures.push_back( current );

    // query program signatures from the database
    set<string>                        names;
    boost::scoped_ptr< Iterator<int> > it( tdb->queryPrograms() );
    while( it->hasNext() ) {
        int pid = it->next();
        signatures.push_back( tdb->querySignature( pid ) );
    }

    // read names
    for( size_t i = 0; i < signatures.size(); ++i ) {
        std::map<std::string, INT64> const& values = signatures[ i ].getValues();
        for( std::map<std::string, INT64>::const_iterator j = values.begin(); j != values.end(); ++j ) {
            names.insert( ( *j ).first );
        }
    }

    // consistency check
    for( size_t i = 0; i < signatures.size(); ++i ) {
        assert( signatures[ i ].size() == names.size() );
    }

    vector<string> result;
    std::copy( names.begin(), names.end(), std::back_inserter( result ) );
    return result;
}

struct svm_node* convertSignature( const vector<string>&   featureName,
                                   ProgramSignature const& signature ) {
    struct svm_node* feature = new struct svm_node[ featureName.size()  + 1 ];
    for( size_t j = 0; j < featureName.size(); j++ ) {
        feature[ j ].index = j;
        feature[ j ].value = signature[ featureName[ j ] ];
    }
    feature[ featureName.size() ].index = -1;
    return feature;
}

std::map<int, struct svm_node*>getAllTrainingFeatures( const vector<string>& featureName ) {
    // query program signatures from the database
    map<int, ProgramSignature>         signatures;
    boost::scoped_ptr< Iterator<int> > it( tdb->queryPrograms() );
    while( it->hasNext() ) {
        int pid = it->next();
        signatures[ pid ] = tdb->querySignature( pid );
    }

    // convert data to result format
    map<int, struct svm_node*> result;
    for( map<int, ProgramSignature>::const_iterator i = signatures.begin(); i != signatures.end(); ++i ) {
        int                     pid       = ( *i ).first;
        ProgramSignature const& signature = ( *i ).second;
        result[ pid ] = convertSignature( featureName, signature );
    }
    return result;
}

/**
 * @brief Converts a TuningConfiguration into value _indices_.
 *
 * @param vs TuningParameter instances are taken from this VariantSpace
 * @param config configuration to convert
 * @return array of index values
 */
std::vector<int>configurationToIndicesInVariantSpace( VariantSpace*              vs,
                                                      TuningConfiguration const& config ) {
    vector<TuningParameter*> tuningParams = vs->getTuningParameters();
    vector<int>              result( tuningParams.size() );

//  for (size_t i = 0; i < tuningParams.size(); i++) {
//    cerr << tuningParams[i]->toString() << endl;
//  }
//  ptree configPtree = config.toPtree();
//  boost::property_tree::write_json(cerr, configPtree);

    for( size_t i = 0; i < tuningParams.size(); i++ ) {
        TuningParameter* param          = tuningParams[ i ];
        int              lackOfValueIdx = -1;

        int idx, val;
        for( idx = 0, val = param->getRangeFrom(); val <= param->getRangeTo(); idx++, val += param->getRangeStep() ) {
            boost::optional<TuningValue> tv0 = param->getTuningValue( val );
            if( !tv0 ) {
                lackOfValueIdx = idx;
                continue;
            }

            if( config.has( tv0.get() ) ) {
                result[ i ] = idx;
                break;
            }
        }

        if( val > param->getRangeTo() ) {
            result[ i ] = lackOfValueIdx;
        }
    }
//  for (size_t i = 0; i < result.size(); i++) {
//    cerr << result[i] << ' ';
//  }
//  cerr << endl;
    return result;
}

std::vector< std::map<int, int> >getAllTrainingLabels( VariantSpace* vs ) {
    size_t                  nTuningParams = vs->getTuningParameters().size();
    vector< map<int, int> > result( nTuningParams );

    boost::scoped_ptr< Iterator<int> > it( tdb->queryPrograms() );
    while( it->hasNext() ) {
        int         pid = it->next();
        vector<int> idx = configurationToIndicesInVariantSpace( vs, tdb->queryBestConfiguration( pid ) );
        assert( idx.size() == nTuningParams );

        for( size_t i = 0; i < nTuningParams; i++ ) {
            if( idx[ i ] != -1 ) {
                result[ i ][ pid ] = idx[ i ];
            }
        }
    }
    return result;
}

double probabilityPrediction( const struct svm_problem*   prob,
                              const struct svm_parameter* param,
                              const struct svm_node*      x,
                              int                         y ) {
    if( const char* msg = svm_check_parameter( prob, param ) ) {
        std::cerr << msg << std::endl;
        return 0.0;
    }

    struct svm_model* model = svm_train( prob, param );
    if( !svm_check_probability_model( model ) ) {
        std::cerr << "Probability prediction was not enabled in SVM parameters." << std::endl;
        return 0.0;
    }

    int nr_class = svm_get_nr_class( model );

    double p[ nr_class ];
    memset( p, 0, sizeof( p ) );
    svm_predict_probability( model, x, p );

    int label[ nr_class ];
    memset( label, 0, sizeof( label ) );
    svm_get_labels( model, label );

    int idx = -1;
    for( int i = 0; i < nr_class; i++ ) {
        if( label[ i ] == y ) {
            idx = i;
        }
    }
    if( idx == -1 ) {
        // std::cerr << "Cannot find class label in model: " << y << std::endl;
        return 0.0;
    }

    svm_free_and_destroy_model( &model );
    return p[ idx ];
}

double probabilityCrossValidation( const struct svm_problem*   prob,
                                   const struct svm_parameter* param ) {
    int                l = prob->l;
    struct svm_problem subprob;
    memset( &subprob, 0, sizeof( subprob ) );
    subprob.l = l - 1;
    subprob.x = new struct svm_node*[ l - 1 ];
    subprob.y = new double[ l - 1 ];

    double sum = 0.0;
    for( int s = 0; s < l; s++ ) {
        for( int i = 0; i < s; i++ ) {
            subprob.x[ i ] = prob->x[ i ];
            subprob.y[ i ] = prob->y[ i ];
        }
        for( int i = s + 1; i < l; i++ ) {
            subprob.x[ i - 1 ] = prob->x[ i ];
            subprob.y[ i - 1 ] = prob->y[ i ];
        }
        sum += probabilityPrediction( &subprob, param, prob->x[ s ], prob->y[ s ] );
    }

    delete[] subprob.x;
    delete[] subprob.y;
    return sum / l;
}

std::vector<double>linearSearch( std::map<int, struct svm_node*> const& x,
                                 std::map<int, int>              const& y,
                                 int m,
                                 const struct svm_node* xp ) {
    struct svm_parameter param;
    memset( &param, 0, sizeof( param ) );
    param.svm_type     = C_SVC;
    param.kernel_type  = LINEAR;
    param.cache_size   = 256;
    param.eps          = 1e-4;
    param.nr_weight    = 0;
    param.weight_label = 0;
    param.weight       = 0;
    param.shrinking    = 0;
    param.probability  = 1;

    int                l = y.size();
    struct svm_problem prob;
    prob.l = l;
    prob.x = new struct svm_node*[ l ];
    prob.y = new double[ l ];

    int                           k;
    map<int, int>::const_iterator it;
    for( k = 0, it = y.begin(); it != y.end(); ++k, ++it ) {
        int pid = ( *it ).first;
        int idx = ( *it ).second;
        prob.x[ k ] = x.find( pid )->second;
        prob.y[ k ] = idx;
    }

    double Cs[] = { 1, 3, 10, 30, 100, 300, 1000, 3000, 10000 };

    double C     = Cs[ 0 ];
    double bestP = 0.0;
    for( int i = 0; i < sizeof( Cs ) / sizeof( Cs[ 0 ] ); i++ ) {
        param.C = Cs[ i ];
        double p = std::max( 1.0 / m, probabilityCrossValidation( &prob, &param ) );
        // fprintf(stderr, "%.6lf\n", p);
        if( p > bestP ) {
            bestP = p;
            C     = param.C;
        }
    }

    std::vector<double> result( m, 0.0 );
    if( bestP > 1.0 / m ) {
        // SVM
        param.C = C;

        if( const char* msg = svm_check_parameter( &prob, &param ) ) {
            std::cerr << msg << std::endl;
            throw msg;
        }

        struct svm_model* model = svm_train( &prob, &param );
        if( !svm_check_probability_model( model ) ) {
            std::cerr << "Probability prediction was not enabled in SVM parameters." << std::endl;
            throw "Probability prediction was not enabled in SVM parameters.";
        }

        int nr_class = svm_get_nr_class( model );

        double p[ nr_class ];
        memset( p, 0, sizeof( p ) );
        svm_predict_probability( model, xp, p );

        int label[ nr_class ];
        memset( label, 0, sizeof( label ) );
        svm_get_labels( model, label );

        for( int i = 0; i < nr_class; i++ ) {
            result[ label[ i ] ] = p[ i ];
        }
        svm_free_and_destroy_model( &model );
    }
    else {
        // uniform
        for( int i = 0; i < m; i++ ) {
            result[ i ] = 1.0 / m;
        }
    }

    delete[] prob.x;
    delete[] prob.y;
    return result;
}
} /* unnamed namespace */


/**
 * Add TuningParameter with cumulative density function (CDF).
 */
void IndependentParametersModel::addParameterWithCDF( TuningParameter*           param,
                                                      const std::vector<double>& CDF ) {
    paramToCDF[ param ] = CDF;
}

/**
 * Add TuningParameter with probability density function (PDF).
 */
void IndependentParametersModel::addParameterWithPDF( TuningParameter*           param,
                                                      const std::vector<double>& PDF ) {
    psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "IndependentParametersModel :: %s\n", param->getName().c_str() );
    for( size_t i = 0; i < PDF.size(); i++ ) {
        psc_dbgmsg( PSC_SELECTIVE_DEBUG_LEVEL( AutotuneSearch ), "\t%d -> %g\n", param->getRangeFrom() + i * param->getRangeStep(), PDF[ i ] );
    }

    vector<double> CDF( PDF.size() );
    CDF.at( 0 ) = PDF.at( 0 );
    for( size_t i = 1; i < CDF.size(); i++ ) {
        CDF[ i ] = CDF[ i - 1 ] + PDF[ i ];
    }

    addParameterWithCDF( param, CDF );
}

/**
 * Add TuningParameter with sample count.
 */
void IndependentParametersModel::addParameterWithSampleCount( TuningParameter*        param,
                                                              const std::vector<int>& sampleCount ) {
    int N = std::accumulate( sampleCount.begin(), sampleCount.end(), 0 );

    vector<double> PDF( sampleCount.size() );
    for( size_t i = 0; i < PDF.size(); i++ ) {
        PDF[ i ] = static_cast<double>( sampleCount[ i ] ) / N;
    }

    addParameterWithPDF( param, PDF );
}


TuningSpecification* IndependentParametersModel::sample() const {
    std::map<TuningParameter*, int> sample_;

    for( TuningParameterToCDF::const_iterator it = paramToCDF.begin(); it != paramToCDF.end(); ++it ) {
        TuningParameter*      tp  = ( *it ).first;
        const vector<double>& CDF = ( *it ).second;

        int                                          k = std::upper_bound( CDF.begin(), CDF.end(), random01() ) - CDF.begin();
        sample_[ tp ] = tp->getRangeFrom() + k * tp->getRangeStep();
    }

    if (withRtsSupport()) {
   	 return new TuningSpecification( new Variant( sample_ ), rtsList );
    } else {
   	 return new TuningSpecification( new Variant( sample_ ), regionList );
    }
}

IndependentParametersModel* IndependentParametersModel::fromConfigurationSamples( SearchSpace*                            ss,
                                                                                  std::vector<TuningConfiguration> const& tcs,
                                                                                  int                                     smoothness ) {
   IndependentParametersModel* model;
	if (!withRtsSupport()){
		vector<Region*>             regions = ss->getRegions();
		model=new IndependentParametersModel(regions);
	} else {
		vector<Rts*>             rtsVec = ss->getRts();
	   model=new IndependentParametersModel(rtsVec);
	}

    std::vector<TuningParameter*> tuningParams = ss->getVariantSpace()->getTuningParameters();
    for( size_t i = 0; i < tuningParams.size(); i++ ) {
        TuningParameter* param = tuningParams[ i ];
        vector<int>      sampleCount;
        int              lackOfValueIdx = -1;

        int idx, val;
        for( idx = 0, val = param->getRangeFrom(); val <= param->getRangeTo(); idx++, val += param->getRangeStep() ) {
            boost::optional<TuningValue> tv0 = param->getTuningValue( val );
            if( !tv0 ) {
                // No TuningValue produced. Save index, and later assign
                // remaining configurations to this value.
                lackOfValueIdx = idx;
                sampleCount.push_back( 0 );
                continue;
            }

            // Count occurrences of TuningValue in the set of TuningConfigurations
            int count = 0;
            for( size_t k = 0; k < tcs.size(); k++ ) {
                if( tcs[ k ].has( tv0.get() ) ) {
                    count++;
                }
            }
            sampleCount.push_back( count );
        }

        // If there was a value, for which getTuningValue did not return
        // a TuningValue, then we shall assign every configuration to that value,
        // where none of the other TuningValues were found.
        if( lackOfValueIdx != -1 ) {
            int sum = std::accumulate( sampleCount.begin(), sampleCount.end(), 0 );
            sampleCount[ lackOfValueIdx ] += std::max<int>( 0, tcs.size() - sum );
        }

        // Apply smoothing
        for( size_t j = 0; j < sampleCount.size(); j++ ) {
            sampleCount[ j ] += smoothness;
        }

        model->addParameterWithSampleCount( param, sampleCount );
    }

    return model;
}

IndependentParametersModel* IndependentParametersModel::learnLinearSVM( SearchSpace*            ss,
                                                                        ProgramSignature const& signature ) {
    vector< map<int, int> > labels = getAllTrainingLabels( ss->getVariantSpace() );
//  for (size_t i = 0; i < labels.size(); i++) {
//    cerr << ss->getVariantSpace()->getTuningParameters()[i]->toString() << endl;
//    for (map<int, int>::const_iterator it = labels[i].begin(); it != labels[i].end(); ++it) {
//      int pid = (*it).first;
//      int idx = (*it).second;
//      cerr << pid << " -> " << idx << endl;
//    }
//  }

    vector<string>             featureNames = getFeatureNames( signature );
    map<int, struct svm_node*> features     = getAllTrainingFeatures( featureNames );
    struct svm_node*           xp           = convertSignature( featureNames, signature );

    vector<TuningParameter*> tuningParam = ss->getVariantSpace()->getTuningParameters();
    vector<int>              nValues( tuningParam.size() );
    for( size_t i = 0; i < nValues.size(); i++ ) {
        TuningParameter* tp = tuningParam[ i ];
        nValues[ i ] = ( tp->getRangeTo() - tp->getRangeFrom() ) / tp->getRangeStep() + 1;
    }

    IndependentParametersModel* result;
    if (!withRtsSupport()){
		 vector<Region*>             regions = ss->getRegions();
		 result  = new IndependentParametersModel(regions);
    } else {
       vector<Rts*>             rtsVec = ss->getRts();
       result  = new IndependentParametersModel(rtsVec);

    }
    for( size_t i = 0; i < labels.size(); i++ ) {
        result->addParameterWithPDF( tuningParam[ i ], linearSearch( features, labels[ i ], nValues[ i ], xp ) );
    }
    return result;
}


IndependentParametersModel::IndependentParametersModel(vector<Region*> regions)
    : regionList( new std::list<Region*>( regions.begin(), regions.end() ) ), random01( boost::mt19937() ) {
    boost::mt19937 rng;
    rng.seed( std::time( 0 ) );
    random01 = boost::uniform_01<boost::mt19937>( rng );
}

IndependentParametersModel::IndependentParametersModel(vector<Rts*> rtsVec)
    : rtsList(new std::list<Rts*>(rtsVec.begin(), rtsVec.end())), random01( boost::mt19937() ) {
    boost::mt19937 rng;
    rng.seed( std::time( 0 ) );
    random01 = boost::uniform_01<boost::mt19937>( rng );
}
