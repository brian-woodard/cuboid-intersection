
#ifndef CUBOID__
#define CUBOID__

#include "Vector.h"
#include <stdio.h>

#define XOUT_YLEFT_ZDOWN 1

class C_cuboid
{
public:
   // Position vector
   C_vector m_vPosition;

   // Orientation matrix
   double m_Yaw;
   double m_pOrientation[3][3];
   enum orientation_axis_index{ X_AXIS, Y_AXIS, Z_AXIS };
   enum orientation_element_index{ X, Y, Z };

   // Height, Width and Depth
   double m_pSize[3];
#if XOUT_YLEFT_ZDOWN
   enum size_index{ DEPTH, WIDTH, HEIGHT };
#else
   enum size_index{ WIDTH, HEIGHT, DEPTH };
#endif

   /****************
    * Constructors *
    ****************/

   //! Constructor C_cuboid()
   //! Default Cuboid Constructor, sets the position to 0s, the sizes to 1, and
   //! the orientation matrix is initialized to an identity matrix.
   C_cuboid( void );

   //! Constructor C_cuboid(C_vector &c)
   //! Cuboid Constructor, sets the position to vector c, the sizes to 1, and
   //! the orientation matrix is initialized to an identity matrix.
   C_cuboid( C_vector &c );

   //! Constructor C_cuboid(double size)
   //! Cuboid Constructor, sets the position to 0s, the sizes to size, and the
   //! orientation matrix is initialized to an identity matrix.
   C_cuboid( double size );

   //! Constructor C_cuboid(double w, double h, double d)
   //! Cuboid Constructor, sets the position to 0s, the sizes to (w, h, d), and
   //! the orientation matrix is initialized to an identity matrix.
   C_cuboid( double w, double h, double d );

   //! Constructor C_cuboid(C_vector &c, double size)
   //! Cuboid Constructor, sets the position to vector c, the sizes size, and
   //! the orientation matrix is initialized to an identity matrix.
   C_cuboid( C_vector &c, double size );

   //! Constructor C_cuboid(C_vector &c, double w, double h, double d)
   //! Cuboid Constructor, sets the position to vector c, the sizes to
   //! (w, h, d), and the orientation matrix is initialized to an identity
   //! matrix.
   C_cuboid( C_vector c, double w, double h, double d );

   /******************
    * Input / Output *
    ******************/

   void get( void );
   void put( void );

   /*************
    * Accessors *
    *************/

   //! C_vector Position()
   //! \details Returns the position of the cuboid.
   //! \return A vector containing the position.
   C_vector Position( void );

   //! double Width()
   //! \details Returns the width of the cuboid.
   //! \return The width in meters.
   double Width( void );

   //! double Height()
   //! \details Returns the height of the cuboid.
   //! \return The height in meters.
   double Height( void );

   //! double Depth()
   //! \details Returns the depth of the cuboid.
   //! \return The depth in meters.
   double Depth( void );

   /*************
    * Modifiers *
    *************/

   //! void SetPosition(C_vector c)
   //! \details Set the center point of the cuboid.
   //! \param[in] c The Flat Earth position of the cuboid.
   void SetPosition( C_vector c );

   //! void SetPosition(double x, double y, double z)
   //! \details Set the center point of the cuboid.
   //! \param[in] x The Flat Earth position x of the cuboid.
   //! \param[in] y The Flat Earth position x of the cuboid.
   //! \param[in] z The Flat Earth position x of the cuboid.
   void SetPosition( double x, double y, double z );

   void operator +=( C_vector &v );
   friend C_cuboid operator +( C_cuboid &c, C_vector &v );

   // Set the height/width/depth vectors
   //   NOTE: This is for updating orientation, these functions assume the length
   //         is remaining the same, to change the length also call the scale functions.

   //! void SetHeight(double h)
   //! \details Sets the height of the cuboid.
   //! \param[in] h The height of the cuboid in meters.
   void SetHeight( double h );

   //! void SetWidth(double w)
   //! \details Sets the width of the cuboid.
   //! \param[in] w The width of the cuboid in meters.
   void SetWidth( double w );

   //! void SetDepth(double d)
   //! \details Sets the depth of the cuboid.
   //! \param[in] d The depth of the cuboid in meters.
   void SetDepth( double d );

   //! void scale(double s)
   //! \details Scale the height/width/depth
   //! \param[in] s The scale factor.
   void scale( double s );
   void operator *=( double s );
   friend C_cuboid operator *( C_cuboid &c, double s );

   //! void Yaw_D(double z)
   //! \details Modify the orientation matrix of the cuboid in heading.
   //! \param[in] z The heading change in degrees.
   void Yaw_D( double z ); // Degrees

   //! void Yaw(double z)
   //! \details Modify the orientation matrix of the cuboid in heading.
   //! \param[in] z The heading change in radians.
   void Yaw( double z ); // Radians

   //! void Pitch_D(double y)
   //! \details Modify the orientation matrix of the cuboid in pitch.
   //! \param[in] y The pitch change in degrees.
   void Pitch_D( double y ); // Degrees

   //! void Pitch(double y)
   //! \details Modify the orientation matrix of the cuboid in pitch.
   //! \param[in] y The pitch change in radians.
   void Pitch( double y ); // Radians

   //! void Roll_D(double x)
   //! \details Modify the orientation matrix of the cuboid in roll.
   //! \param[in] x The roll change in degrees.
   void Roll_D( double x ); // Degrees

   //! void Roll(double x)
   //! \details Modify the orientation matrix of the cuboid in roll.
   //! \param[in] x The roll change in radians.
   void Roll( double x ); // Radians

   //! void SetYaw_D(double z)
   //! \details Sets the orientation matrix of the cuboid heading.
   //! \param[in] z The heading in degrees.
   void SetYaw_D( double z ); // Degrees

   //! void SetYaw(double z)
   //! \details Sets the orientation matrix of the cuboid heading.
   //! \param[in] z The heading in radians.
   void SetYaw( double z ); // Radians

   //! void SetPitch_D(double y)
   //! \details Sets the orientation matrix of the cuboid pitch.
   //! \param[in] y The pitch in degrees.
   void SetPitch_D( double y ); // Degrees

   //! void SetPitch(double y)
   //! \details Sets the orientation matrix of the cuboid pitch.
   //! \param[in] y The pitch in radians.
   void SetPitch( double y ); // Radians

   //! void SetRoll_D(double x)
   //! \details Sets the orientation matrix of the cuboid roll.
   //! \param[in] x The roll in degrees.
   void SetRoll_D( double x ); // Degrees

   //! void SetRoll(double x)
   //! \details Sets the orientation matrix of the cuboid roll.
   //! \param[in] x The roll in radians.
   void SetRoll( double x ); // Radians

   /***********************
    * Collision Detection *
    ***********************/

   //! double SphereCollision(C_vector &pos, double rad)
   //! \details If the supplied Sphere is NOT within the cuboids boundary
   //!          sphere then the distance between the Sphere's edge and the
   //!          closest possible distance to the cuboid center is returned. If
   //!          the supplied Sphere is within the cuboids boundary sphere but
   //!          not colliding with the cuboid itself the distance between the
   //!          Sphere's edge and closest face is returned. If collision has
   //!          occurred then 0.0 is returned.
   //! \param[in] pos The position of the sphere.
   //! \param[in] rad The radius of the sphere.
   //! \return The distance between the sphere edge and the closest point on
   //!         this cuboid, a value of 0.0 indicates a collision.
   int SphereCollision( const C_vector &pos, double rad, double& miss_distance, C_vector& poc );

   int SphereCollisionOld( const C_vector &pos, double rad, double& miss_distance, C_vector& poc );

   void GetFaceCorners(int Face, C_vector& C1, C_vector& C2, C_vector& C3, C_vector& C4);
};

#endif//CUBOID__
