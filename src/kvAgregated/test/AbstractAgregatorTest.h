#ifndef ABSTRACTAGREGATORTEST_H_
#define ABSTRACTAGREGATORTEST_H_

#include <gtest/gtest.h>
#include <AbstractAgregator.h>
#include <boost/shared_ptr.hpp>


typedef boost::shared_ptr<agregator::AbstractAgregator> AgregatorPtr;

class AbstractAgregatorTest : public testing::TestWithParam<AgregatorPtr>
{};

#endif /*ABSTRACTAGREGATORTEST_H_*/
