/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file   CameraSet.h
 * @brief  Base class to create smart factors on poses or cameras
 * @author Frank Dellaert
 * @date   Feb 19, 2015
 */

#pragma once

#include <gtsam/geometry/Point3.h>
#include <gtsam/geometry/CalibratedCamera.h> // for Cheirality exception
#include <vector>

namespace gtsam {

/**
 * @brief A set of cameras, all with their own calibration
 * Assumes that a camera is laid out as 6 Pose3 parameters then calibration
 */
template<class CAMERA>
class CameraSet {

private:

  std::vector<CAMERA> cameras_;

  /**
   * 2D measurement and noise model for each of the m views
   * The order is kept the same as the keys that we use to create the factor.
   */
  typedef typename CAMERA::Measurement Z;

  static const int ZDim = traits<Z>::dimension; ///< Measurement dimension
  static const int Dim = traits<CAMERA>::dimension; ///< Camera dimension

  /// shorthand for this class
  typedef CameraSet<CAMERA> This;

public:

  /// Default Constructor
  CameraSet() {
  }

  /** Virtual destructor */
  virtual ~CameraSet() {
  }

  /**
   * Add a new camera
   */
  void add(const CAMERA& camera) {
    cameras_.push_back(camera);
  }

  /**
   * print
   * @param s optional string naming the factor
   * @param keyFormatter optional formatter useful for printing Symbols
   */
  void print(const std::string& s = "") const {
    std::cout << s << "CameraSet, cameras = \n";
    for (size_t k = 0; k < cameras_.size(); ++k)
      cameras_[k]->print();
  }

  /// equals
  virtual bool equals(const CameraSet& p, double tol = 1e-9) const {
    bool camerasAreEqual = true;
    for (size_t i = 0; i < cameras_.size(); i++) {
      if (cameras_.at(i).equals(p.cameras_.at(i), tol) == false)
        camerasAreEqual = false;
      break;
    }
    return camerasAreEqual;
  }

  /**
   *  project, with derivatives in this, point, and calibration
   * throws CheiralityException
   */
  std::vector<Z> project(const Point3& point, boost::optional<Matrix&> F=boost::none,
      boost::optional<Matrix&> E=boost::none, boost::optional<Matrix&> H=boost::none) const {

    size_t nrCameras = cameras_.size();
    if (F) F->resize(ZDim * nrCameras, 6);
    if (E) E->resize(ZDim * nrCameras, 3);
    if (H && Dim>6) H->resize(ZDim * nrCameras, Dim - 6);
    std::vector<Z> z(nrCameras);

    for (size_t i = 0; i < cameras_.size(); i++) {
      Eigen::Matrix<double, ZDim, 6> Fi;
      Eigen::Matrix<double, ZDim, 3> Ei;
      Eigen::Matrix<double, ZDim, Dim - 6> Hi;
      z[i] = cameras_[i].project(point, F ? &Fi : 0, E ? &Ei : 0, H ? &Hi : 0);
      if (F) F->block<ZDim, 6>(ZDim * i, 0) = Fi;
      if (E) E->block<ZDim, 3>(ZDim * i, 0) = Ei;
      if (H) H->block<ZDim, Dim - 6>(ZDim * i, 0) = Hi;
    }
    return z;
  }

private:

  /// Serialization function
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & cameras_;
  }
};

template<class CAMERA>
const int CameraSet<CAMERA>::ZDim;

} // \ namespace gtsam
