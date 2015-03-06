(function (angular) {
    /// TEMPLATES

    angular.module("pills.templates", []).run(["$templateCache", function($templateCache) {
        $templateCache.put("pill-services.tmpl.html",
            '<div watering service="vm.servicesStatuses.Svc_Watering"></div>' +
            '<div kinect service="vm.servicesStatuses.Svc_Kinect"></div>' +
            '<div httplistener service="vm.servicesStatuses.Svc_HttpListener"></div>' +
            '<div adminui service="vm.servicesStatuses.Svc_AdminUI"></div>');

        $templateCache.put("pill-services/watering.tmpl.html", [
            '<div>' +
                '<div class="col-lg-8">' +
                    '<div class="well clearfix">',
                        '<div class="row">' +
                            '<div class="col-lg-4">' +
                                '<div class="input-group  input-group-sm inputGroup">'+
                                '<span class="input-group-addon" id="basic-addon1">Amount (ml)</span>'+
                                '<input  name="input" type="number" min="0" max="10000" required class="form-control waterAmountInput" ng-model="manualVisibleLevel"/>' +
                                '<span class="error" ng-show="AmountInputForm.input.$error.number"> Not valid number!</span>'  +
                                '</div>'+
                            '<p><a class="btn btn-default btn-sm" ng-click="onClickInput()" >Input</a></p>'+

                        //'<div style="height: 80px;" class="progress progress-striped  progress-vertical-hack">' +
                        //'<div class="progress-bar progress-bar-info active" role="progressbar" aria-valuenow="{{getPercentage()}}" aria-valuemin="0" aria-valuemax="100" ' +
                        //    'ng-style="{width : ( getPercentage() + \'%\' ) }">' +
                        //'<span class="sr-only">45% Complete</span>' +
                        //'</div>' +
                        //'</div>' +

                            '</div>' +
                            '<div class="col-lg-8">' +
                                '<pre> {{service | json}} </pre>' +
                            '</div>' +
                        '</div>' +
                    '</div>' +
                '</div>' +
            '</div>'
        ].join(''));

        $templateCache.put("pill-services/httplistener.tmpl.html", [
            '<div><div class="col-lg-4"><div class="well">',
                '<h2>HttpListener Service<span ng-class="[status.css]">{{status.label}}</span></h2>',
                '<p><a class="btn btn-default btn-sm">Secret Button</a></p><pre> {{service | json}} </pre></div>'+
            '</div></div>'
        ].join(''));

        $templateCache.put("pill-services/adminui.tmpl.html", [
            '<div><div class="col-lg-4"><div class="well">',
                '<h2>AdminUI Service<span ng-class="[status.css]">{{status.label}}</span></h2>',
                '<p><a class="btn btn-default btn-sm">Secret Button</a></p><pre> {{service | json}} </pre></div>'+
            '</div></div>'
        ].join(''));

        $templateCache.put("pill-services/kinect.tmpl.html", [
            '<div><div class="col-lg-4"><div class="well">',
                '<h2>Kinect Service<span ng-class="[status.css]">{{status.label}}</span></h2>',
                '<p><a class="btn btn-default btn-sm">Secret Button</a></p><pre> {{service | json}} </pre></div>'+
            '</div></div>'
        ].join(''));

        $templateCache.put("settings/settings-root.tmpl.html", [
            '<div class="col-lg-12">',
            '<h2>System Configuration Setup</h2>',
            '<p/>' +
            '<div class="row">',
                '<div class="col-md-4 wizard-steps">',
                '<div class="wizard-nav-container">',
                '<ul class="nav nav-pills nav-stacked" style="padding-bottom:30px;">',
                '<li ng-class="{\'disabled\': !vm.steps.step1.canGo()}" ui-sref-active="active"><a ui-sref=".step1" eat-click-if="vm.steps.step1.canGo()" class="wizard-nav-link"><i class="icon-chevron-right"></i>Pumps Calibration</a></li>',
                '<li ng-class="{\'disabled\': !vm.steps.step2.canGo()}" ui-sref-active="active"><a ui-sref=".step2" eat-click-if="vm.steps.step2.canGo()" class="wizard-nav-link"><i class="icon-chevron-right"></i>Configuration acceptance</a></li>',
                //'<li ng-class="{\'disabled\': !vm.steps.step3.canGo()}" ui-sref-active="active"><a ui-sref=".step3" eat-click-if="vm.steps.step3.canGo()" class="wizard-nav-link"><i class="icon-chevron-right"></i>Notification Schedule</a></li>',
                '</ul>',
                '</div>',
                '<div class="wizard-progress-container"><div class="progress progress-striped active"><div class="progress-bar progress-bar-success" ng-style="{ \'width\': (vm.steps.current-1)/vm.steps.total*100 + \'%\' }" style="width: 33%;"></div></div></div>',
                '</div>',
                '<div class="col-md-8" ui-view="step"></div>',
                '</div>',

                '<div class="alert alert-danger" ng-show="vm.showError()">' +
                '{{vm.errorText}}' +
                '</div>' +

            '</div>'
        ].join(''));

        $templateCache.put("settings/settings-step1.tmpl.html", [
            '<h1>Pumps calibration</h1>' +
            '<p> 1. Prepare your installation. Once everything is installed, connected and verified, press Next to open watering pump which is needed for calculating your pumps performance. </p>' +
            '<p> 2. Once a water leaks the appropriate hosepipe, press Stop button. </p>' +

            '<div class="panel panel-default"> '+
                '<div class="panel-heading">'+
                    '<h3 class="panel-title">Pumps calibration</h3>'+
                '</div>'+

                '<div class="panel panel-body"> '+
                    '<div class="well well-sm">'+
                        '<div class="btn-group btn-group-xm" role="group" aria-label="...">'+
                            '<button class="btn btn-default" ng-click="vm.steps.step1.startButtonClicked(1)" ng-class="{\'disabled\': !vm.steps.step1.startButtonEnabled(1)}"> Start Pump 1</button>' +
                            '<button class="btn btn-default" ng-click="vm.steps.step1.stopButtonClicked(1)" ng-class="{\'disabled\': !vm.steps.step1.stopButtonEnabled(1)}"> Stop Pump 1</button>' +
                        '</div>' +
                        '<span class="resultLabel">Working time: {{ vm.steps.step1.workingTime(1) }}</span>' +

                        '<div class="input-group  input-group-sm inputGroup">'+
                            '<span class="input-group-addon" id="basic-addon1">Water pumped (ml)</span>'+
                            '<input  name="input" ng-disabled="!vm.steps.step1.inputEnabled(1)" type="number" min="0" max="10000" required class="form-control waterAmountInput" id="exampleInputName2" ng-model="vm.pumpData.pump1.waterPumpedAmount"/>' +
                            '<span class="error" ng-show="AmountInputForm.input.$error.number"> Not valid number!</span>'  +
                        '</div>'+
                    '</div>  '+

                    '<div class="well well-sm">'+
                        '<div class="btn-group btn-group-xm" role="group" aria-label="...">'+
                            '<button class="btn btn-default" ng-click="vm.steps.step1.startButtonClicked(2)" ng-class="{\'disabled\': !vm.steps.step1.startButtonEnabled(2)}"> Start Pump 2</button>' +
                            '<button class="btn btn-default" ng-click="vm.steps.step1.stopButtonClicked(2)" ng-class="{\'disabled\': !vm.steps.step1.stopButtonEnabled(2)}"> Stop Pump 2</button>' +
                        '</div>' +
                        '<span class="resultLabel">Working time: {{ vm.steps.step1.workingTime(2) }}</span>' +

                        '<div class="input-group  input-group-sm inputGroup">'+
                            '<span class="input-group-addon" id="basic-addon1">Water pumped (ml)</span>'+
                            '<input  name="input" ng-disabled="!vm.steps.step1.inputEnabled(2)" type="number" min="0" max="10000" required class="form-control waterAmountInput" id="exampleInputName2" ng-model="vm.pumpData.pump2.waterPumpedAmount"/>' +
                            '<span class="error" ng-show="AmountInputForm.input.$error.number"> Not valid number!</span>'  +
                        '</div>'+
                    '</div>  '+
                '</div>  '+
            '</div>  '+

            //'<button class="btn btn-danger" ng-click="vm.steps.step1.warmup()" ng-class="{\'disabled\': !vm.steps.step1.canGoToStrep2()}"> Coo </button>' +
            '<a role="button" ng-disabled="!vm.steps.step1.completionCheck()" eat-click-if="vm.steps.step2.canGo()" ui-sref="^.step2" type="button" class="btn btn-success" ng-click="vm.steps.step1.completedCallback()">Next</button>'
        ].join(''));

        $templateCache.put("settings/settings-step2.tmpl.html", [
            '<h1>Configuration acceptance</h1>',
            '<div class="well">' +
                '<pre contenteditable="true"> {{vm.configurationJsonDocument | json:4}} </pre>' +
            '</div>' +
            '<a role="button" eat-click-if="vm.steps.step1.canGo()" ui-sref="^.step1" type="button" class="btn btn-default">Previous</button>',
            '<a role="button"  ng-click="vm.steps.step2.uploadClicked()" ng-disabled="!vm.steps.step2.uploadAllowed()" type="button" class="btn btn-success">Upload</button>',
            '<a role="button"  ui-sref="app" type="button" class="btn btn-success">Finish</button>',
        ].join(''));
        //
        //$templateCache.put("settings/settings-step3.tmpl.html", [
        //    '<h1>Step 3</h1>',
        //    '<a role="button" eat-click-if="vm.steps.step2.canGo()" ui-sref="^.step2" type="button" class="btn btn-success">Previous</button>',
        //    '<a role="button" ng-disabled="!vm.steps.step3.completionCheck()" ui-sref="app" type="button" class="btn btn-success" ng-click="vm.steps.step3.completedCallback()">Finish</button>'
        //].join(''));
    }]);

    /// CONFIG

    angular.module('pills', [
        'pills.dependency', 'pills.config', 'pills.router', 'pills.controllers', 'pills.templates',
        'pills.services', 'pills.settings', 'pills.header'
    ]);

    angular.module('pills.dependency', ['ui.router', 'ngResource', 'eatClickIf']);

    angular.module('pills').config(['$locationProvider', '$httpProvider', '$urlRouterProvider', function($locationProvider, $httpProvider, $urlRouterProvider){
        $httpProvider.defaults.useXDomain = true;
        delete $httpProvider.defaults.headers.common['X-Requested-With'];
        $httpProvider.defaults.headers.post['Content-Type'] = 'application/json;charset=utf-8';
        $httpProvider.defaults.useXDomain = true;
        $locationProvider.html5Mode(false);
        $urlRouterProvider.otherwise('/');
    }]);

    angular.module('pills.config', []).constant('config', {
        api: "http://127.0.0.1:9999/",
        calibrationServiceApi: "http://127.0.0.1:4567/",
        statuses: {
            ready:{value:0, label:'Ready', css:"text-success"},
            working:{value:1, label:'Working..', css:"text-warning"},
            error:{value:2, label:'Error', css:"text-danger"}
        }
    });

    angular.module('pills.router', []).config(['$stateProvider', function ($stateProvider) {
        $stateProvider.state('app', {
            url: '/',
            views: {
                '@': {
                    templateUrl: 'pill-services.tmpl.html',
                    controller: 'PillServicesController as vm'
                }
            }
        }).state('wizard', {
            url: '/settings',
            views: {
                '@': {
                    templateUrl: 'settings/settings-root.tmpl.html',
                    controller: 'SettingsController as vm'
                }
            }
        }).state('wizard.step1', {
            url: '/step1',
            views: {
                'step': {
                    templateUrl: 'settings/settings-step1.tmpl.html',
                }
            },
            data: {
                step: 1
            }
        }).state('wizard.step2', {
            url: '/step2',
            views: {
                'step': {
                    templateUrl: 'settings/settings-step2.tmpl.html',
                }
            },
            data: {
                step: 2
            }});
        //}).state('wizard.step3', {
        //    url: '/step3',
        //    views: {
        //        'step': {
        //            templateUrl: 'settings/settings-step3.tmpl.html',
        //        }
        //    },
        //    data: {
        //        step: 3
        //    }
        //});
    }]);

    // ----------------------------------------------------------------------------------------------------------------------------------------------------------
    // Dashboard controller
    // ----------------------------------------------------------------------------------------------------------------------------------------------------------
    angular.module('pills.controllers', []).controller('PillServicesController', ['$scope', 'CalibrationService', '$interval', function PillServicesController($scope, CalibrationService, $interval) {
        var _this = this;

        _this.servicesStatuses = {};
        var intevalTimer = $interval(function() {
            CalibrationService.clusterStatus().then(function(data) {
                _this.servicesStatuses = data;
            }, function(err) {
                _this.servicesStatuses = null;
            });
        }, 300);

        $scope.$on(
            "$destroy",
            function handleDestroyEvent() {
                $interval.cancel(intevalTimer);
            }
        );

    }]);

    angular.element(document).ready(function () {
        angular.bootstrap(document, ['pills'], {
            strictDi: true
        });
    });

    /// SETTINGS
    angular.module('pills.settings', ['pills.settings.controller']);
    angular.module('pills.settings.controller', ['pills.calibration']).controller('SettingsController', ['$scope', '$state', '$rootScope', 'CalibrationService',
        function SettingsController($scope, $state, $rootScope, CalibrationService) {
            var _this = this;

            this.settings = {
                value1: '',
                value2: '',
                value3: '',
                value4: ''
            };

            //var resultId = null;

            this.errorText = "";
            this.showError = function () {
                return _this.errorText !== "";
            };

            var stateChangeCancelation = $rootScope.$on('$stateChangeStart', function (event, toState, toParams, fromState, fromParams) {
                if (toState.name.indexOf('wizard') == -1) {
                    stateChangeCancelation();
                } else {
                    if (angular.isUndefined(toState.data) || angular.isUndefined(toState.data.step)) {
                        $state.go('wizard.step1');
                        return;
                    }
                    if (!(_this.steps['step' + toState.data.step].canGo())) {
                        event.preventDefault();
                        return;
                    }
                    _this.steps.current = toState.data.step;
                }
            });

            if (angular.isUndefined($state.$current.data) || angular.isUndefined($state.$current.data.step)) {
                $state.go('wizard.step1', { reload: true });
                return;
            }

            var pumpData = {
                pump1: {
                    workingTime: null,
                    state: 'stopped',
                    intervalTimer: null,
                    complete: false,
                    waterPumpedAmount: 0
                },
                pump2: {
                    workingTime: null,
                    state: 'stopped',
                    intervalTimer: null,
                    complete: false,
                    waterPumpedAmount: 0
                },

                select: function(pumpNumber) {
                    return pumpData['pump' + pumpNumber];
                }
            };
            this.pumpData = pumpData;
            this.configurationJsonDocument = {
                pumps: {
                    inputPump: {
                        performance: null
                    },
                    outputPump: {
                        performance: null
                    }
                },
                "containers":
                {
                    "visibleContainer":
                    {
                        "shape": "rectangular",
                        "height": 100,
                        "width": 30,
                        "depth": 65
                    },

                    "hiddenContainer":
                    {
                        "shape": "rectangular",
                        "height": 110,
                        "width": 80,
                        "depth": 35
                    }
                },

                "pumpOutMap":
                {
                    "levels":
                        [
                            {
                                "levelHeight": 10,
                                "velocityLitresPerHour": 0
                            },
                            {
                                "levelHeight": 10,
                                "velocityLitresPerHour": 2
                            },
                            {
                                "levelHeight": 10,
                                "velocityLitresPerHour": 80
                            },
                            {
                                "levelHeight": 10,
                                "velocityLitresPerHour": 120
                            },
                            {
                                "levelHeight": 10,
                                "velocityLitresPerHour": 150
                            },
                            {
                                "levelHeight": 15,
                                "velocityLitresPerHour":  600
                            }
                        ]
                }
            };

            this.steps =
            {
                current: $state.$current.data.step || 1,
                total: 2,

                step1: {
                    inputEnabled: function(pumpNumber) {
                        return pumpData.select(pumpNumber).complete;
                    },

                    canGo: function() {
                        return true;
                    },

                    completionCheck: function() {
                        return (pumpData.select(1).waterPumpedAmount > 0) && (pumpData.select(2).waterPumpedAmount > 0);
                    },

                    completedCallback: function() {
                        _this.steps.step1.completed = true;

                        var pump1PerformanceMlPerHr = Math.round(pumpData.select(1).waterPumpedAmount / (pumpData.select(1).workingTime / 1000) * 3600);
                        var pump2PerformanceMlPerHr = Math.round(pumpData.select(2).waterPumpedAmount / (pumpData.select(2).workingTime / 1000) * 3600);
                        _this.configurationJsonDocument.pumps.inputPump.performance = pump1PerformanceMlPerHr;
                        _this.configurationJsonDocument.pumps.outputPump.performance = pump2PerformanceMlPerHr;
                        _this.configurationJsonDocument.pumpOutMap.levels[_this.configurationJsonDocument.pumpOutMap.levels.length - 1].velocityLitresPerHour = pump1PerformanceMlPerHr / 1000;
                    },

                    startButtonClicked: function(pumpNumber) {
                        if (!CalibrationService.isConnectionOk()) {
                            console.log('Failed starting the pump. Backend/Hardware service connection problem');
                            return;
                        }
                        pumpData.select(pumpNumber).state = 'started';
                        pumpData.select(pumpNumber).complete = false;
                        pumpData.select(pumpNumber).waterPumpedAmount = 0;

                        var charIndex = 0;
                        pumpData.select(pumpNumber).intervalTimer = setInterval(function () {
                            $scope.$apply(function() { pumpData.select(pumpNumber).workingTime = ['|','/','\u2012','\\'][charIndex % 4]; });
                            charIndex += 1;
                        }, 75);

                        CalibrationService.startPump(pumpNumber).then(function (data) {
                            console.log('Pump seems to be started');
                        }, function (error) {
                            _this.errorText = "Failed starting pump: " + JSON.stringify(error);
                        });
                    },

                    stopButtonClicked: function(pumpNumber) {
                        if (!CalibrationService.isConnectionOk()) {
                            console.log('Failed stopping the pump. Backend/Hardware service connection problem');
                            return;
                        }
                        CalibrationService.stopPump(pumpNumber).then(function (data) {
                            if (pumpData.select(pumpNumber).intervalTimer !== null) {
                                clearInterval(pumpData.select(pumpNumber).intervalTimer);
                                pumpData.select(pumpNumber).intervalTimer = null;
                            }
                            pumpData.select(pumpNumber).workingTime = data.result.workingTimeMs;
                            pumpData.select(pumpNumber).state = 'stopped';
                            pumpData.select(pumpNumber).complete = true;

                        }, function (error) {
                            _this.errorText = "Failed stopping pump: " + JSON.stringify(error);
                        });

                        _this.errorText = "";
                    },

                    stopButtonEnabled: function(pumpNumber) {
                        return pumpData.select(pumpNumber).state === 'started';
                    },
                    startButtonEnabled: function(pumpNumber) {
                        return pumpData.select(pumpNumber).state === 'stopped' && CalibrationService.isConnectionOk();
                    },

                    canGoToStrep2: function() {
                        return true;
                    },

                    workingTime: function(pumpNumber){
                        return pumpData.select(pumpNumber).workingTime === null
                            ? '---'
                            : pumpData.select(pumpNumber).workingTime;
                    },

                    completed: false
                },
                step2: {
                    uploadClicked: function() {
                        CalibrationService.uploadConfiguration(JSON.stringify(_this.configurationJsonDocument)).then(function(data) {
                            console.log('config uploaded successfully');
                        },
                        function (error) {
                            console.log('failed uploading configuration');
                        });
                    },
                    uploadAllowed: function() {
                        return CalibrationService.isConnectionOk();
                    },
                    canGo: function() {
                        return _this.steps.step1.completed;
                    },
                    completionCheck: function() {
                        return true;
                    },
                    completedCallback: function() {
                        _this.steps.step2.completed = true;
                    },
                    completed: false
                },
                step3: {
                    canGo: function() {
                        return _this.steps.step2.completed;
                    },
                    completionCheck: function() {
                        return true;
                    },
                    completedCallback: function() {
                        _this.steps.step3.completed = true;
                    },
                    completed: false
                }
            };

            if (!(_this.steps['step' + _this.steps.current].canGo())) {
                $state.go('wizard.step1', { reload: true });
                return;
            }
    }]);

    // ------------------------------------------------------------------------------------------------------------------------------------
    // MenuController
    // ------------------------------------------------------------------------------------------------------------------------------------
    angular.module('pills.header', []).controller('MenuController',['CalibrationService', function (CalibrationService) {
        var _this= this;

        this.isBackEndOnline = function (){
            return CalibrationService.getStatus().status === 0;
        };
        this.isHardwareOnline = function (){
            return CalibrationService.getStatus().hardStatus === 0;
        };
    }]);

    // ------------------------------------------------------------------------------------------------------------------------------------
    // Callibration service
    // ------------------------------------------------------------------------------------------------------------------------------------
    angular.module('pills.calibration', []).factory('CalibrationService', ['$q', '$resource', 'config', function ($q, $resource, config) {

        var hardwareServiceResource = $resource(buildUrl('pumpControl/:action'), null, {
            startPump: { method: 'POST', params: { action:'start'} },
            stopPump: { method: 'POST', params: { action: 'stop'} }
        });

        var statusResource = $resource(buildUrl('status'), null, {
            getStatus: { method: 'GET' }
        });

        var stopResult = $resource(buildUrl('stop_result/:resultId'), { resultId:'@resultId' }, {
            getResult: { method: 'GET' }
        });

        var clusterStatusResource = $resource(buildUrl('cluster_status'), null, {
            clusterStatus: { method: 'GET', isArray: false }
        });

        var configurationResource = $resource(buildUrl('upload_config'), null, {
           uploadConfig: { method: 'POST'}
        });

        var debugInterfaceResource = $resource(buildUrl('debug_cmds/:action'), null, {
            setVisibleLevel: { method: 'POST', params: {action: 'setVisible'}}
        });

        var currentStatus = {status: 0, hardStatus: 0};

        // Status poller
        setInterval(function () {
            statusResource.getStatus().$promise.then(function (data) {
                console.log("Got a status: " + JSON.stringify(data));
                currentStatus = data;
            }, function (error) {
                console.log("Failed getting status");
                currentStatus = {status: 1, hardStatus: 1};
            });

        }, 500);

        var _public = {
            dbgSetVisibleWaterLevel: function(value) {
                return debugInterfaceResource.setVisibleLevel({amount: value}).$promise;
            },
            uploadConfiguration: function(configJsonText) {
                return configurationResource.uploadConfig({configJsonText: configJsonText}).$promise;
            },
            clusterStatus: function() {
                return clusterStatusResource.clusterStatus().$promise;
            },
            startPump: function(pumpId) {
                return hardwareServiceResource.startPump({pumpId: pumpId}).$promise;
            },
            stopPump: function (pumpId) {
                var deferred = $q.defer();
                 hardwareServiceResource.stopPump({pumpId: pumpId}).$promise.then(function (data) {
                    resultId = data.resultId;
                    //// start result polling
                    var pollingFunction = function () {
                        _public.getStopResult(data.resultId).then(function (response) {
                            // got stop result, check it
                            if (response.status === 'not_ready') {
                                console.log('restart poller..');
                                setTimeout(pollingFunction, 100);
                            } else {
                                deferred.resolve(response);
                            }
                        }, function (error) {
                            // failed getting stop result
                            console.log('restart poller due to error..');
                            setTimeout(pollingFunction, 100);
                        });
                    };

                    // Try to get results after 100 msecs after request
                    setTimeout(pollingFunction, 0);

                }, function (error) {
                     deferred.reject(error); // (will send control to error part of .then clause)
                });
                return deferred.promise;
            },
            getStatus: function() {
                return currentStatus;
            },
            isConnectionOk: function() {
                return currentStatus.status === 0 && currentStatus.hardStatus === 0;
            },
            getStopResult: function (resultId) {
                return stopResult.getResult({}, { resultId: resultId }).$promise;
            }
        };

        return _public;

        function buildUrl(resourceUrl, domain) {
            if (resourceUrl.lastIndexOf('/') !== resourceUrl.length - 1) {
                resourceUrl += "/";
            }

            return (domain ? domain : config.calibrationServiceApi) + resourceUrl;
        }
    }]);

    /// WATERING

    angular.module('pills.services', ['pills.watering.directive', 'pills.httplistener.directive', 'pills.adminui.directive', 'pills.kinect.directive']);
    angular.module('pills.watering.directive', []).directive('watering', ['CalibrationService', function (CalibrationService){
        return {
            resrict: 'A',
            replace: true,
            scope: { service: '=service' },
            templateUrl: 'pill-services/watering.tmpl.html',
            controller: function($scope) {
                
                $scope.onClickInput = function () {
                    CalibrationService.dbgSetVisibleWaterLevel($scope.manualVisibleLevel);
                };
                

                //$scope.$watch("service.status.details.input.visibleLevel", function(newValue, oldValue) {
                //    console.log('oldvalue: ' + oldValue + ', newValue: ' + newValue);
                //    if (angular.isDefined(newValue )) {
                //        $scope.visibleActualLevel = newValue;
                //
                //        if ($scope.manualVisibleLevel === undefined)
                //            $scope.manualVisibleLevel = $scope.visibleActualLevel;
                //    }
                //    else {
                //        $scope.visibleActualLevel = 0;
                //        //$scope.visibleActualLevel
                //    }
                //
                //});

                // var levelMax = 65;
                //
                //$scope.getPercentage = function () {
                //    var result = (($scope.visibleActualLevel / levelMax) * 100).toFixed(2);
                //    return result;
                //};
                //
                //$scope.$watch("manualVisibleLevel", function(newValue, oldValue) {
                //    if (newValue == undefined) {
                //        console.log('undefined !!!');
                //    }
                //    console.log('oldvalue: ' + oldValue + ', newValue: ' + newValue);
                //    if (newValue !== undefined)
                //        CalibrationService.dbgSetVisibleWaterLevel(newValue);
                //});
            }
        }
    }]);
    angular.module('pills.httplistener.directive', []).directive('httplistener', [function (){
        return {
            resrict: 'A',
            replace: true,
            scope: { service: '=service' },
            templateUrl: 'pill-services/httplistener.tmpl.html'
        }
    }]);
    angular.module('pills.adminui.directive', []).directive('adminui', [function (){
        return {
            resrict: 'A',
            replace: true,
            scope: { service: '=service' },
            templateUrl: 'pill-services/adminui.tmpl.html'
        }
    }]);
    angular.module('pills.kinect.directive', []).directive('kinect', [function (){
        return {
            resrict: 'A',
            replace: true,
            scope: { service: '=service' },
            templateUrl: 'pill-services/kinect.tmpl.html'
        }
    }]);

    /// EatClickIf
    angular.module('eatClickIf', []).directive('eatClickIf', [
        '$parse', '$rootScope', function($parse, $rootScope) {
            return {
                priority: 100,
                restrict: 'A',
                compile: function($element, attr) {
                    var fn = $parse(attr.eatClickIf);
                    return {
                        pre: function link(scope, element) {
                            var eventName = 'click';
                            element.on(eventName, function(event) {
                                var callback = function() {
                                    if (!fn(scope, { $event: event })) {
                                        event.stopImmediatePropagation();
                                        event.preventDefault();
                                        event.stopPropagation();
                                        return false;
                                    }
                                };
                                if ($rootScope.$$phase) {
                                    scope.$evalAsync(callback);
                                } else {
                                    scope.$apply(callback);
                                }
                            });
                        }
                    }
                }
            }
        }
    ]);
})(angular);