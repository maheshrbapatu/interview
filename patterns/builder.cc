class File {
    string name;
    int permission; 
    friend class BuildFile;
};

class BuildFile {
    File f;
    public: 
        BuildFile& setName(string name) {
            f.name = name;
            return *this;
        }
        
        BuildFile& setPermission(int perm) {
            f.permission = perm;
            return *this;
        }
        
        File build() {
            return f;
        }
};
