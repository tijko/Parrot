pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                sh 'apt install -y libprocps-dev'
                sh 'apt install -y glib-dbus'
                sh 'apt install -y dbus-glib'
                sp 'apt install -y pkg-config'
                sh 'make'
            }
        }
        stage('Deploy') {
            steps {
                sh 'make install'
            }
        }
    }
}
