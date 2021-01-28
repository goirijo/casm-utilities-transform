#include <iostream>
#include <casmutils/xtal/structure.hpp>
#include <casmutils/xtal/structure_tools.hpp>
#include <CLI/CLI.hpp>
#include <vector>

Eigen::Matrix3d read_matrix(const std::filesystem::path& input_path)
{
    std::ifstream mat_stream(input_path);

    Eigen::Matrix3d mat;
    for(int i=0; i<3; ++i)
    {
        for(int j=0; j<3; ++j)
        {
            mat_stream>>mat(i,j);
        }
    }

    mat_stream.close();
    return mat;
}

int main(int argc, char* argv[])
{
    namespace cu=casmutils;

    // Parse CLI
    CLI::App app;

    // Path to input structure
    cu::fs::path input_struc_path;
    cu::fs::path input_mat_path;
    cu::fs::path output_struc_path; //default output to screen
    std::vector<double> input_rotation_axis;
    double input_rotation_angle;
 
    //TODO: Add option to enable printing input
    app.add_option("-s,--structure",input_struc_path,"Path to input structure (vasp format).")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output",output_struc_path,"Path to write transformed structure to. Defaults to printing to screen if no output path indicated");
    auto* matrix_opt = app.add_option("-m,--matrix",input_mat_path,"Path to file containing 3x3 transformation matrix.")->check(CLI::ExistingFile);
    auto* rotation_opt = app.add_option("-a,--axis",input_rotation_axis,"Rotation axis in vector form")->expected(3);
    //TODO: Make this degrees
    auto* rotation_ang_opt = app.add_option("-r,--angle",input_rotation_angle,"Rotation angle in radians");
    
    rotation_opt->needs(rotation_ang_opt)->excludes(matrix_opt);
    rotation_ang_opt->needs(rotation_opt)->excludes(matrix_opt);
    matrix_opt->excludes(rotation_opt)->excludes(rotation_ang_opt);

    CLI11_PARSE(app, argc, argv);

    //debugging
    std::cout << "read input structure\n";
    // Read a structure
    auto input_struc=cu::xtal::Structure::from_poscar(input_struc_path);

    //debugging
    std::cout << "Checking empty output path: " << output_struc_path.empty() << "\n";
    
    //TODO: if no transformation throw warning message
    if(!((rotation_opt->count())||matrix_opt->count())) {
        std::cout << "WARNING: No transformation specified\n";
    }

    //Writes input to screen as a check
    std::cout <<"Your input structure was:\n";
    cu::xtal::print_poscar(input_struc,std::cout);
    
    if(rotation_opt->count()) {
    //Read rotation inputs and normalize the axis
    Eigen::Vector3d rotation_axis;
    for(int i = 0; i<3; i++) {
        rotation_axis(i)=input_rotation_axis[i];
    }
    
    //debugging
    std::cout << "normalizing rotation axis\n";
    
    rotation_axis.normalize();

    //create rotation_matrix (in proper format) - check syntax for Eigen 3d matrix
    Eigen::AngleAxisd rotation_AngleAxis(input_rotation_angle,rotation_axis);
    Eigen::Matrix3d rotation_matrix = rotation_AngleAxis.matrix();
    
    //debugging
    std::cout << "applying rotation\n";
    
    //Apply rotation to lattice
    auto final_struc=cu::xtal::apply_deformation(input_struc, rotation_matrix);
    
    //debugging
    std::cout << "rotation applied\n";

    // Output transformed structure
    if(!(output_struc_path.empty())) {
        cu::xtal::write_poscar(final_struc,output_struc_path);
        }
    else {
        std::cout << "No output path specified, printing to screen:\n";
        cu::xtal::print_poscar(final_struc,std::cout);
        }
    
    }

    else {
    // Read a transformation matrix
    Eigen::Matrix3d transform_mat=read_matrix(input_mat_path);
    
    //debugging
    std::cout << "matrix was read \n";

    // Apply transformation to lattice
    auto final_struc=cu::xtal::apply_deformation(input_struc, transform_mat);
    
    // Output transformed structure
    if(!(output_struc_path.empty())) {
        cu::xtal::write_poscar(final_struc,output_struc_path);
        }
    else {
        std::cout << "No output path specified, printing to screen:\n";
        cu::xtal::print_poscar(final_struc,std::cout);
        }
    }

    return 0;
}